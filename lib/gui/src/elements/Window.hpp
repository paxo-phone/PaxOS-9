//
// Created by Gabriel on 12/02/2024.
//

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "../ElementBase.hpp"

#include <any>
#include <map>

namespace gui::elements
{
    class Window final : public ElementBase
    {
        // Use the metadate to store information about the window.
        std::map<std::string, std::any> metadata_;

      public:
        Window();
        ~Window() override;

        static std::vector<Window*> windows;

        void render() override;

        void widgetUpdate() override;

        std::any getMetadata(const std::string& key);
        void setMetadata(const std::string& key, const std::any& value);
    };
} // namespace gui::elements

#endif // WINDOW_HPP
