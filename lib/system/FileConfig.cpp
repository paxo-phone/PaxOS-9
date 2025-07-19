//
// Created by Charles on 08/09/2024.
//

#include "FileConfig.hpp"

#include <filestream.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <path.hpp>

namespace libsystem
{
    FileConfig::FileConfig(const storage::Path& path)
    {
        m_path = path;

        // Init fields before parsing
        m_version = 0;
        m_mainNode = std::make_shared<NamespaceNode>("main");
        m_currentNode = m_mainNode;

        m_fileStream = nullptr;
        m_fileStreamSize = 0;
        m_fileStreamCursor = 0;

        if (m_path.exists())
        {
            // If file exist, read it and parse it
            // Else, do nothing

            // Open file in binary mode (FileStream is always binary)
            openFileStream(m_path, storage::READ);

            if (!checkFormat())
                throw exceptions::RuntimeError("File is the wrong format.");

            parse();

            closeFileStream();
        }
    }

    FileConfig::~FileConfig() = default;

    void FileConfig::write()
    {
        openFileStream(m_path, storage::WRITE);

        m_fileStream->write("BFC");
        m_fileStream->write(static_cast<int8_t>(m_version));

        m_mainNode->write(m_fileStream);

        closeFileStream();
    }

    bool FileConfig::has(const std::string& key) const
    {
        std::vector<std::string> keySplit = getSplicedNamespacedKey(key);

        std::shared_ptr<NamespaceNode> namespaceNode = m_mainNode;
        while (keySplit.size() > 1)
        {
            if (const std::string namespaceName = keySplit.front();
                namespaceNode->hasNamespace(namespaceName))
            {
                namespaceNode = namespaceNode->getNamespace(namespaceName);
            }
            else
            {
                return false;
            }

            keySplit.erase(keySplit.begin());
        }

        return namespaceNode->hasValue(keySplit[0]);
    }

    FileConfig::file_config_types_t FileConfig::getRaw(const std::string& key) const
    {
        const std::shared_ptr<NamespaceNode> namespaceNode = getNamespaceNodeFromNamespaceKey(key);

        // Get only the last part of the key
        const std::string keySuffix = key.substr(key.find_last_of('.') + 1);

        const std::shared_ptr<ValueNode> value = namespaceNode->getValue(keySuffix);

        return value->getValue();
    }

    void FileConfig::setRaw(const std::string& key, const file_config_types_t& value) const
    {
        const std::shared_ptr<NamespaceNode> namespaceNode =
            getNamespaceNodeFromNamespaceKey(key, true);

        // keySuffix: Get only the last part of the key
        if (const std::string keySuffix = key.substr(key.find_last_of('.') + 1);
            namespaceNode->hasValue(keySuffix))
        {
            // Update value
            const std::shared_ptr<ValueNode> valueNode = namespaceNode->getValue(keySuffix);

            valueNode->setValue(value);
        }
        else
        {
            // Create value
            const auto valueNode = std::make_shared<ValueNode>(keySuffix, value);

            namespaceNode->addValueNode(valueNode);
        }
    }

    std::string FileConfig::toString() const
    {
        return "FileConfig{path=" + m_path.str() + ", version=" + std::to_string(m_version) + "}";
    }

    bool FileConfig::checkFormat()
    {
        return readString(3) == "BFC";
    }

    void FileConfig::parse()
    {
        m_version = readUint8();

        OpCode opCode;

        while (hasNext() && (opCode = readOpCode()) != NULL_CODE)
        {
            switch (opCode)
            {
            case PROPERTY:
                newProperty();
                break;
            case BEGIN_NAMESPACE:
                pushNamespace();
                break;
            case END_NAMESPACE:
                popNamespace();
                break;
            default:;
            }
        }
    }

    void FileConfig::newProperty()
    {
        const Type type = readType();
        const std::string key = readString();

        file_config_types_t value;

        switch (type)
        {
        case UINT8:
            value = readUint8();
            break;
        case UINT16:
            value = readUint16();
            break;
        case UINT32:
            value = readUint32();
            break;
        case UINT64:
            value = readUint64();
            break;
        case STRING:
            value = readString();
            break;
        case INT:
            value = static_cast<int>(readUint32());
            break;
        case FLOAT:
        {
            uint32_t intValue = readUint32();
            float floatValue = *reinterpret_cast<float*>(&intValue);
            value = floatValue;
            break;
        }
        case DOUBLE:
        {
            uint64_t intValue = readUint64();
            double doubleValue = *reinterpret_cast<double*>(&intValue);

            value = doubleValue;

            break;
        }
        case BOOL:
            value = static_cast<bool>(readUint8());
            break;
        case LIST:
            value = static_cast<std::vector<std::string>>(readList());
            break;
        default:;
        }

        std::cout << "Key: " << key << std::endl;

        const auto valueNode = std::make_shared<ValueNode>(key, value);
        m_currentNode->addValueNode(valueNode);
    }

    void FileConfig::pushNamespace()
    {
        const std::string name = readString();

        if (m_currentNode->hasNamespace(name))
        {
            // If namespace already exists
            // Get existing namespace
            const std::shared_ptr<NamespaceNode> namespaceNode = m_currentNode->getNamespace(name);
            m_currentNode = namespaceNode;
            return;
        }

        // If namespace doesn't exist
        // Create new one
        const auto namespaceNode = std::make_shared<NamespaceNode>(name);
        m_currentNode->addNamespaceNode(namespaceNode);
        m_currentNode = namespaceNode;
    }

    void FileConfig::popNamespace()
    {
        m_currentNode = m_currentNode->getParent();
    }

    std::string FileConfig::getNamespacedKey(const std::string& key) const
    {
        if (m_currentNode->getParent() == nullptr)
            return key;

        return m_currentNode->getPath() + "." + key;
    }

    // ReSharper disable once CppDFAUnreachableFunctionCall
    std::vector<std::string> FileConfig::getSplicedNamespacedKey(const std::string& namespacedKey)
    {
        std::vector<std::string> output;

        // Code from :
        //  https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c

        size_t next = 0;
        size_t last = 0;
        while ((next = namespacedKey.find('.', last)) != std::string::npos)
        {
            output.push_back(namespacedKey.substr(last, next - last));
            last = next + 1;
        }

        output.push_back(namespacedKey.substr(last));

        return output;
    }

    // ReSharper disable once CppDFAUnreachableFunctionCall
    std::shared_ptr<FileConfig::NamespaceNode> FileConfig::getNamespaceNodeFromNamespaceKey(
        const std::string& namespacedKey, const bool createNewNamespaces
    ) const
    {

        std::vector<std::string> keySplit = getSplicedNamespacedKey(namespacedKey);

        std::shared_ptr<NamespaceNode> namespaceNode = m_mainNode;
        while (keySplit.size() > 1)
        {
            if (const std::string namespaceName = keySplit.front();
                namespaceNode->hasNamespace(namespaceName))
            {
                namespaceNode = namespaceNode->getNamespace(namespaceName);
            }
            else if (createNewNamespaces)
            {
                auto newNamespace = std::make_shared<NamespaceNode>(namespaceName);
                namespaceNode->addNamespaceNode(newNamespace);
                namespaceNode = newNamespace;
            }
            else
            {
                throw exceptions::InvalidArgument("Unknown key: " + namespacedKey + ".");
            }

            keySplit.erase(keySplit.begin());
        }

        return namespaceNode;
    }

    void FileConfig::openFileStream(const storage::Path& path, storage::Mode mode)
    {
        m_fileStream = std::make_shared<storage::FileStream>(path.str(), mode);
        m_fileStreamSize = m_fileStream->size();
        m_fileStreamCursor = 0;

        // Reset cursor
        m_fileStream->close();
        m_fileStream->open(path.str(), mode);
    }

    void FileConfig::closeFileStream()
    {
        m_fileStream->close();
        m_fileStream.reset();
    }

    bool FileConfig::hasNext() const
    {
        return m_fileStreamCursor < m_fileStreamSize;
    }

    uint8_t FileConfig::read()
    {
        m_fileStreamCursor++;

        return m_fileStream->readchar();
    }

    uint8_t FileConfig::readUint8()
    {
        return read();
    }

    uint16_t FileConfig::readUint16()
    {
        return read() << 8 | read();
    }

    uint32_t FileConfig::readUint32()
    {
        return read() << 24 | read() << 16 | read() << 8 | read();
    }

    uint64_t FileConfig::readUint64()
    {
        // #pragma GCC diagnostic push
        // #pragma GCC diagnostic ignored "-Wshift-count-overflow"

        const uint64_t r =
            static_cast<uint64_t>(read()) << 56 | static_cast<uint64_t>(read()) << 48 |
            static_cast<uint64_t>(read()) << 40 | static_cast<uint64_t>(read()) << 32 |
            static_cast<uint64_t>(read()) << 24 | static_cast<uint64_t>(read()) << 16 |
            static_cast<uint64_t>(read()) << 8 | static_cast<uint64_t>(read());

        return r;

        // #pragma GCC diagnostic pop
    }

    std::string FileConfig::readString(const size_t length)
    {
        std::string output;

        if (length == 0)
        {
            char c;
            while (hasNext() && (c = static_cast<char>(read())) != 0x00) output += c;
        }
        else
        {
            for (size_t i = 0; i < length; i++) output += m_fileStream->readchar();
        }

        return output;
    }

    std::vector<std::string> FileConfig::readList()
    {
        std::vector<std::string> output;

        if (m_fileStream->readchar() != '[')
            throw std::invalid_argument("FileConfig List Error");

        std::string lst;
        std::string var;
        char c;

        do {
            c = m_fileStream->readchar();
            if (c == EOF)
            {
                std::cerr << "[FileConfig] invalid List format" << std::endl;
                break;
            }
            if (c == 0x00)
            {
                output.push_back(lst);
                lst = "";
            }
            else
            {
                lst += c;
            }
        } while (c != ']');
        return output;
    }

    FileConfig::OpCode FileConfig::readOpCode()
    {
        return static_cast<OpCode>(read());
    }

    FileConfig::Type FileConfig::readType()
    {
        return static_cast<Type>(read());
    }

    FileConfig::ValueNode::ValueNode(const std::string& key, const file_config_types_t& value)
    {
        m_key = key;
        m_value = value;
    }

    std::shared_ptr<FileConfig::NamespaceNode> FileConfig::ValueNode::getParent() const
    {
        return std::static_pointer_cast<NamespaceNode>(m_parent);
    }

    std::string FileConfig::ValueNode::getKey()
    {
        return m_key;
    }

    FileConfig::Type FileConfig::ValueNode::getType() const
    {
        return static_cast<Type>(m_value.index()); // Risky
    }

    FileConfig::file_config_types_t FileConfig::ValueNode::getValue()
    {
        return m_value;
    }

    void FileConfig::ValueNode::setValue(const file_config_types_t& value)
    {
        m_value = value;
    }

    std::string FileConfig::ValueNode::getPath()
    {
        if (getParent() == nullptr)
            return m_key;

        return getParent()->getPath() + "." + m_key;
    }

    void FileConfig::ValueNode::write(const std::shared_ptr<storage::FileStream>& fileStream) const
    {
        fileStream->write(PROPERTY);
        fileStream->write(getType());

        fileStream->write(m_key);
        fileStream->write(0x00);

        switch (getType())
        {
        case UINT8:
            fileStream->write(static_cast<char>(std::get<uint8_t>(m_value)));
            break;
        case UINT16:
        {
            const uint16_t v = std::get<uint16_t>(m_value);

            fileStream->write(static_cast<char>(v >> 8));
            fileStream->write(static_cast<char>(v & 0xFF));

            break;
        }
        case UINT32:
        {
            const uint32_t v = std::get<uint32_t>(m_value);

            fileStream->write(static_cast<char>(v >> 24));
            fileStream->write(static_cast<char>(v >> 16 & 0xFF));
            fileStream->write(static_cast<char>(v >> 8 & 0xFF));
            fileStream->write(static_cast<char>(v & 0xFF));

            break;
        }
        case UINT64:
        {
            const uint64_t v = std::get<uint64_t>(m_value);

            fileStream->write(static_cast<char>(v >> 56));
            fileStream->write(static_cast<char>(v >> 48 & 0xFF));
            fileStream->write(static_cast<char>(v >> 40 & 0xFF));
            fileStream->write(static_cast<char>(v >> 32 & 0xFF));
            fileStream->write(static_cast<char>(v >> 24 & 0xFF));
            fileStream->write(static_cast<char>(v >> 16 & 0xFF));
            fileStream->write(static_cast<char>(v >> 8 & 0xFF));
            fileStream->write(static_cast<char>(v & 0xFF));

            break;
        }
        case STRING:
        {
            const std::string v = std::get<std::string>(m_value);

            for (const char c : v) fileStream->write(c);
            fileStream->write(0x00);

            break;
        }
        case INT:
        {
            const int v = std::get<int>(m_value);
            const auto v32 = static_cast<uint32_t>(v);

            fileStream->write(static_cast<char>(v32 >> 24));
            fileStream->write(static_cast<char>(v32 >> 16 & 0xFF));
            fileStream->write(static_cast<char>(v32 >> 8 & 0xFF));
            fileStream->write(static_cast<char>(v32 & 0xFF));

            break;
        }
        case FLOAT:
        {
            const float v = std::get<float>(m_value);
            const auto v32 = *reinterpret_cast<const uint32_t*>(&v);

            fileStream->write(static_cast<char>(v32 >> 24));
            fileStream->write(static_cast<char>(v32 >> 16 & 0xFF));
            fileStream->write(static_cast<char>(v32 >> 8 & 0xFF));
            fileStream->write(static_cast<char>(v32 & 0xFF));

            break;
        }
        case DOUBLE:
        {
            const double v = std::get<double>(m_value);
            const auto v64 = *reinterpret_cast<const uint64_t*>(&v);

            fileStream->write(static_cast<char>(v64 >> 56));
            fileStream->write(static_cast<char>(v64 >> 48 & 0xFF));
            fileStream->write(static_cast<char>(v64 >> 40 & 0xFF));
            fileStream->write(static_cast<char>(v64 >> 32 & 0xFF));
            fileStream->write(static_cast<char>(v64 >> 24 & 0xFF));
            fileStream->write(static_cast<char>(v64 >> 16 & 0xFF));
            fileStream->write(static_cast<char>(v64 >> 8 & 0xFF));
            fileStream->write(static_cast<char>(v64 & 0xFF));

            break;
        }
        case BOOL:
        {
            const bool v = std::get<bool>(m_value);

            // ReSharper disable once CppRedundantCastExpression
            fileStream->write(static_cast<char>(v));

            break;
        }
        case LIST:
        {
            const std::vector<std::string> v = std::get<std::vector<std::string>>(m_value);
            fileStream->write("[");
            for (const std::string str : v)
            {
                for (const char c : str) fileStream->write(c);
                fileStream->write(0x00);
                break;
            }
            fileStream->write("]");
        }

        default:
            fileStream->write(0x00);
        }
    }

    FileConfig::NamespaceNode::NamespaceNode(const std::string& name)
    {

        m_name = name;
    }

    std::shared_ptr<FileConfig::NamespaceNode> FileConfig::NamespaceNode::getParent() const
    {
        return std::static_pointer_cast<NamespaceNode>(m_parent);
    }

    std::string FileConfig::NamespaceNode::getName()
    {
        return m_name;
    }

    bool FileConfig::NamespaceNode::hasValue(const std::string& key)
    {
        const auto it = m_values.find(key);
        return it != m_values.end();
    }

    std::shared_ptr<FileConfig::ValueNode>
        FileConfig::NamespaceNode::getValue(const std::string& key)
    {
        const auto it = m_values.find(key);

        if (it == m_values.end())
            throw exceptions::RuntimeError("Cannot find value.");

        return it->second;
    }

    bool FileConfig::NamespaceNode::hasNamespace(const std::string& name)
    {
        const auto it = m_namespaces.find(name);
        return it != m_namespaces.end();
    }

    std::shared_ptr<FileConfig::NamespaceNode>
        FileConfig::NamespaceNode::getNamespace(const std::string& name)
    {
        const auto it = m_namespaces.find(name);

        if (it == m_namespaces.end())
            throw exceptions::RuntimeError("Cannot find namespace.");

        return it->second;
    }

    void FileConfig::NamespaceNode::addValueNode(const std::shared_ptr<ValueNode>& node)
    {
        node->m_parent = shared_from_this();
        m_values.insert({node->getKey(), node});
    }

    void FileConfig::NamespaceNode::addNamespaceNode(const std::shared_ptr<NamespaceNode>& node)
    {
        node->m_parent = shared_from_this();
        m_namespaces.insert({node->getName(), node});
    }

    std::string FileConfig::NamespaceNode::getPath()
    {
        std::string path = m_name;

        std::shared_ptr<NamespaceNode> parent;
        while ((parent = getParent()) != nullptr)
        {
            path.insert(0, ".");
            path.insert(0, parent->getName());
        }

        return path;
    }

    void FileConfig::NamespaceNode::write(
        const std::shared_ptr<storage::FileStream>& fileStream
    ) const
    { // NOLINT(*-no-recursion)
        // Write begin namespace OpCode, if not top-level namespace
        if (getParent() != nullptr)
        {
            fileStream->write(BEGIN_NAMESPACE);
            fileStream->write(m_name);
            fileStream->write(0x00);
        }

        // Write every namespace recursively
        for (const auto& [fst, snd] : m_namespaces) snd->write(fileStream);

        // Write every values
        for (const auto& [fst, snd] : m_values) snd->write(fileStream);

        // Write end namespace OpCode, if not top-level namespace
        if (getParent() != nullptr)
            fileStream->write(END_NAMESPACE);
    }
} // namespace libsystem
