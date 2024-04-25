#include "Canvas.hpp"
#include <iostream>

namespace gui::elements
{
    Canvas::Canvas(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        this->m_x = x;
        this->m_y = y;
        this->m_width = width;
        this->m_height = height;
    }

    void Canvas::render() {}

    void Canvas::setPixel(int16_t x, int16_t y, color_t color)
    {
        this->getAndSetSurface()->setPixel(x, y, color);
        this->localGraphicalUpdate();
    }

    void Canvas::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color)
    {
        this->getAndSetSurface()->drawRect(x, y, w, h, color);
        this->localGraphicalUpdate();
    }

    void Canvas::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color)
    {
        this->getAndSetSurface()->fillRect(x, y, w, h, color);
        this->localGraphicalUpdate();
    }

    void Canvas::drawCircle(int16_t x, int16_t y, uint16_t radius, color_t color)
    {
        this->getAndSetSurface()->drawCircle(x, y, radius, color);
        this->localGraphicalUpdate();
    }

    void Canvas::fillCircle(int16_t x, int16_t y,  uint16_t radius, color_t color)
    {
        this->getAndSetSurface()->fillCircle(x, y, radius, color);
        this->localGraphicalUpdate();
    }

    void Canvas::drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color)
    {
        this->getAndSetSurface()->drawRoundRect(x, y, w, h, radius, color);
        this->localGraphicalUpdate();
    }

    void Canvas::fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color)
    {
        this->getAndSetSurface()->fillRoundRect(x, y, w, h, radius, color);
        this->localGraphicalUpdate();
    }
        
    void Canvas::drawPolygon(std::vector<std::pair<int16_t, int16_t>> vertices, color_t color)
    {
        if (vertices.empty())
        {
            return;
        }

        std::pair<int16_t, int16_t> firstPosition = vertices[0];

        std::pair<int16_t, int16_t> currentPosition = firstPosition;

        for (std::pair<int16_t, int16_t> vertex : vertices)
        {
            this->drawLine(currentPosition.first, currentPosition.second, vertex.first, vertex.second, color);
            currentPosition = vertex;
        }

        // draw the last line
        this->drawLine(currentPosition.first, currentPosition.second, firstPosition.first, firstPosition.second, color);        
    }

    void Canvas::fillPolygon(std::vector<std::pair<int16_t, int16_t>> vertices, color_t color)
    {        
        if (vertices.empty())
        {
            return;
        }

        for (std::pair<int16_t, int16_t> vertex : vertices)
        {
            std::cout << "Vertex: " << vertex.first << ", " << vertex.second << std::endl;
        }

        // assuming the polygon is convex
        point_t topVertex = *std::max_element(vertices.begin(), vertices.end(), [](const point_t& lhs, const point_t& rhs) { return lhs.second > rhs.second; });
        point_t bottomVertex = *std::max_element(vertices.begin(), vertices.end(), [](const point_t& lhs, const point_t& rhs) { return lhs.second < rhs.second; });

        size_t verticesCount = vertices.size();

        // for each line in the polygon, from the top to the bottom
        for (int16_t y = topVertex.second; y < bottomVertex.second; y++)
        {
            const int16_t k_minimumXIntersection = this->m_width - 1;
            const int16_t k_maximumXIntersection = 0;

            int16_t minimumXIntersection = k_minimumXIntersection;
            int16_t maximumXIntersection = maximumXIntersection; // the maximum value they could take

            for (uint16_t i = 0; i < verticesCount; i++)
            {
                point_t vertex1 = vertices[i];
                point_t vertex2 = vertices[(i + 1) % verticesCount /* to get the first vertex if we reach the end of the vector */];

                if (vertex1.second > vertex2.second)
                {
                    std::swap(vertex1, vertex2);
                }

                if (y >= vertex1.second && y < vertex2.second) // avoid division by zero
                {
                    // calculate the x coordinate of the intersection point of the line between vertex1 and vertex2 with the horizontal line at y
                    uint16_t xIntersection = vertex1.first + (y - vertex1.second) * (vertex2.first - vertex1.first) / (vertex2.second - vertex1.second);
                    if (xIntersection < minimumXIntersection)
                    {
                        minimumXIntersection = xIntersection;
                    }
                    if (xIntersection > maximumXIntersection)
                    {
                        maximumXIntersection = xIntersection;
                    }

                    if (minimumXIntersection <= k_maximumXIntersection && maximumXIntersection >= k_minimumXIntersection)
                    { // no need to continue iterating, we already known that the whole line has to be filled
                        break;
                    }
                }
            }

            this->drawLine(minimumXIntersection, y, maximumXIntersection, y, color);
        }
    }

    void Canvas::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color)
    {
        this->getAndSetSurface()->drawLine(x1, y1, x2, y2, color);
        this->localGraphicalUpdate();
    }

    Canvas::~Canvas() = default;
}