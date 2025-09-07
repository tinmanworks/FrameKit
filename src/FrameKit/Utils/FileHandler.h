#pragma once

#include "fkpch.h"
#include <fstream>

namespace FrameKit {
    inline  std::string CalculateHash(const std::string& input) {
        // Use std::hash to hash the input string
        size_t hashValue = std::hash<std::string>{}(input);


        // Convert the hash value to a hexadecimal string
        std::stringstream hashStream;
        hashStream << std::hex << std::setw(16) << std::setfill('0') << hashValue;

        return hashStream.str();
    }

    inline std::string TruncateFileExtension(const std::string& filename) {
        size_t lastDot = filename.find_last_of(".");
        if (lastDot != std::string::npos) {
            return filename.substr(0, lastDot);
        }
        return filename; // No file extension found
    }

    class File {
    private:
        Ref<char[]> m_FileData;
        size_t m_FileSize = 0;
    public:

        File() :m_FileData(nullptr), m_FileSize(0) {}
        ~File() {

        }
        size_t GetFileSize() {
            return m_FileSize;
        }

        Ref<char[]> GetData() {
            return m_FileData;
        }
        Ref<char[]> ReadTextFile(const std::filesystem::path& filePath) {

            if (!std::filesystem::exists(filePath)) {
                return nullptr;
            }

            std::ifstream f(filePath);
            if (!f.is_open()) {
                return nullptr;
            }



            f.seekg(0, std::ios::end);
            const size_t fileSize = f.tellg();

            if (fileSize > 0) {

                f.seekg(0, std::ios::beg); // Reset file position to the beginning
                m_FileData = CreateRef<char[]>(fileSize);
                f.read(m_FileData.get(), fileSize);
                f.close();
                m_FileSize = fileSize;
            }

            return m_FileData;
        }

        Ref<char[]> ReadBinFile(const std::filesystem::path& filePath) {
            if (!std::filesystem::exists(filePath)) {
                return nullptr;
            }

            std::ifstream f(filePath, std::ios::binary);
            if (!f.is_open()) {
                return nullptr;
            }

            f.seekg(0, std::ios::end);
            const size_t fileSize = f.tellg();

            if (fileSize > 0) {

                f.seekg(0, std::ios::beg); // Reset file position to the beginning
                m_FileData = CreateRef<char[]>(fileSize);
                f.read(m_FileData.get(), fileSize);
                f.close();
                m_FileSize = fileSize;
            }
            return m_FileData;
        }

        void WriteBinFile(const std::filesystem::path filePath, const char* data, const size_t dataSize) {
            std::ofstream file(filePath, std::ios::binary);
            file.write(data, dataSize);
            file.close();

        }

        void WriteTextFile(std::filesystem::path filePath, const char* data, const size_t dataSize) {
            std::ofstream file(filePath);
            file.write(data, dataSize);
            file.close();
        }

        template<typename T>
        void WriteBinFile(const std::filesystem::path filePath, const T& data) {
            size_t dataSize = sizeof(T);
            std::ofstream file(filePath, std::ios::binary);
            file.write(data, dataSize);
            file.close();

        }
    };
}

