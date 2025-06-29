//
// Created by Charles on 08/09/2024.
//

#ifndef FILECONFIG_HPP
#define FILECONFIG_HPP

#include <cstdint>
#include <filestream.hpp>
#include <map>
#include <memory>
#include <path.hpp>
#include <string>
#include <variant>
#include <vector>

namespace libsystem
{
    /**
     * @brief Class to store user preferences in a binary format.
     *
     * File config is a binary file format (.bfc).
     * It is used to store simple data like user preferences (E.g. screen
     * brightness). Should be faster than JSON.
     *
     * @todo Write better FileStream implementation.
     *
     * @todo Add lists support.
     */
    class FileConfig
    {
        // Need to match "FileConfig::Type" !
        /**
         * Available types for values.
         * Logic needs to be implemented before adding any new value.
         */

      public:
        typedef std::variant<
            std::nullptr_t, uint8_t, uint16_t, uint32_t, uint64_t, std::string, int, float, double,
            bool, std::vector<std::string>>
            file_config_types_t;

        /**
         * Create and load a file config.
         * @param path The path to load and store the config.
         */
        explicit FileConfig(const storage::Path& path);
        ~FileConfig();

        /**
         * Write the cached data to disk.
         */
        void write();

        [[nodiscard]] bool has(const std::string& key) const;

        // Can't implement in .cpp
        /**
         * Get a value from the configuration.
         * @tparam T The type of the value to get.
         * @param key The key of the value to get.
         * @return The value.
         */
        template <typename T> [[nodiscard]] T get(const std::string& key) const
        {
            return std::get<T>(getRaw(key));
        }

        // Can't implement in .cpp
        /**
         * Set a value in the configuration.
         * @tparam T The type of the value to set.
         * @param key The key of the value to set.
         * @param value The value to set to.
         */
        template <typename T> void set(const std::string& key, T value) const
        {
            setRaw(key, static_cast<T>(value));
        }

        /**
         * Simple conversion for debug purposes, don't show any contained value.
         * @return The string version of the object.
         */
        [[nodiscard]] std::string toString() const;

      private:
        enum OpCode
        {
            NULL_CODE,
            PROPERTY,
            BEGIN_NAMESPACE,
            END_NAMESPACE
        };

        enum Type
        {
            NULL_TYPE,
            UINT8,
            UINT16,
            UINT32,
            UINT64,
            STRING,
            INT,
            FLOAT,
            DOUBLE,
            BOOL,
            LIST
        };

        enum VersionFlag
        {
            EMPTY
        };

        class Node : public std::enable_shared_from_this<Node>
        {
          public:
            virtual ~Node() = default;

            virtual std::string getPath() = 0;

          protected:
            std::shared_ptr<Node> m_parent = nullptr;
        };

        class NamespaceNode;

        class ValueNode final : public Node
        {
            friend class NamespaceNode;

          public:
            ValueNode(const std::string& key, const file_config_types_t& value);

            [[nodiscard]] std::shared_ptr<NamespaceNode> getParent() const;

            std::string getKey();
            Type getType() const;

            file_config_types_t getValue();
            void setValue(const file_config_types_t& value);

            std::string getPath() override;

            void write(const std::shared_ptr<storage::FileStream>& fileStream) const;

          private:
            std::string m_key;
            file_config_types_t m_value;
        };

        class NamespaceNode final : public Node
        {
            friend class ValueNode;

          public:
            explicit NamespaceNode(const std::string& name);

            [[nodiscard]] std::shared_ptr<NamespaceNode> getParent() const;

            std::string getName();

            bool hasValue(const std::string& key);
            std::shared_ptr<ValueNode> getValue(const std::string& key);

            bool hasNamespace(const std::string& name);
            std::shared_ptr<NamespaceNode> getNamespace(const std::string& name);

            void addValueNode(const std::shared_ptr<ValueNode>& node);
            void addNamespaceNode(const std::shared_ptr<NamespaceNode>& node);

            std::string getPath() override;

            void write(const std::shared_ptr<storage::FileStream>& fileStream) const;

          private:
            std::string m_name;
            std::map<std::string, std::shared_ptr<NamespaceNode>> m_namespaces;
            std::map<std::string, std::shared_ptr<ValueNode>> m_values;
        };

        storage::Path m_path;

        std::shared_ptr<storage::FileStream> m_fileStream;
        size_t m_fileStreamSize;
        size_t m_fileStreamCursor;

        uint8_t m_version;

        std::shared_ptr<NamespaceNode> m_mainNode;
        std::shared_ptr<NamespaceNode> m_currentNode;

        /**
         * Check the provided file format.
         * @return True if the opened file is the correct format.
         */
        [[nodiscard]] bool checkFormat();

        void parse();

        /**
         * Read a property and store it.
         */
        void newProperty();

        /**
         * Read a namespace and push it in the namespace stack.
         */
        void pushNamespace();

        /**
         * Pop the last namespace from the namespace stack.
         */
        void popNamespace();

        /**
         * Returns the key processed with the namespace stack.
         * @param key The key to namespace.
         * @return The namespaced key.
         */
        [[nodiscard]] std::string getNamespacedKey(const std::string& key) const;

        /**
         * Split a namespaced key to a vector.
         * @param namespacedKey The key to split.
         * @return A vector containing every namespace before the key (also
         * included).
         */
        static std::vector<std::string> getSplicedNamespacedKey(const std::string& namespacedKey);

        [[nodiscard]] std::shared_ptr<NamespaceNode> getNamespaceNodeFromNamespaceKey(
            const std::string& namespacedKey, bool createNewNamespaces = false
        ) const;

      public:
        [[nodiscard]] file_config_types_t getRaw(const std::string& key) const;

      private:
        void setRaw(const std::string& key, const file_config_types_t& value) const;

        /*
         * FileStream interface
         */

        void openFileStream(const storage::Path& path, storage::Mode mode);
        void closeFileStream();

        [[nodiscard]] bool hasNext() const;

        // Big endian
        [[nodiscard]] uint8_t read();

        [[nodiscard]] uint8_t readUint8();
        [[nodiscard]] uint16_t readUint16();
        [[nodiscard]] uint32_t readUint32();
        [[nodiscard]] uint64_t readUint64();
        [[nodiscard]] std::string readString(size_t length = 0);
        [[nodiscard]] std::vector<std::string> readList();

        [[nodiscard]] OpCode readOpCode();
        [[nodiscard]] Type readType();
    };
} // namespace libsystem

#endif // FILECONFIG_HPP
