#include "File.h"
#include "Utils.h"
#include <fstream>

File::File(const std::string& name, AccessLevel level, Resource* parent,
    const std::string& ext, size_t sz, const std::string& cont)
    : Resource(name, level, parent), extension(ext), size(sz), content(cont) {
    if (!Utils::isValidExtension(ext))
        throw InvalidNameException("Invalid extension: " + ext);
}

void File::setExtension(const std::string& ext) {
    if (!Utils::isValidExtension(ext))
        throw InvalidNameException("Invalid extension: " + ext);
    extension = ext;
}

void File::setSize(size_t sz) { size = sz; }
void File::setContent(const std::string& cont) { content = cont; }

std::unique_ptr<Resource> File::clone() const {
    auto newFile = std::make_unique<File>(name, accessLevel, nullptr, extension, size, content);
    newFile->creationDate = Utils::getCurrentDate(); // новая дата
    return newFile;
}

void File::serialize(std::ofstream& out) const {
    int typeInt = static_cast<int>(Type::File);
    out.write(reinterpret_cast<const char*>(&typeInt), sizeof(typeInt));
    serializeBase(out);

    size_t extLen = extension.size();
    out.write(reinterpret_cast<const char*>(&extLen), sizeof(extLen));
    out.write(extension.c_str(), extLen);

    out.write(reinterpret_cast<const char*>(&size), sizeof(size));

    size_t contLen = content.size();
    out.write(reinterpret_cast<const char*>(&contLen), sizeof(contLen));
    out.write(content.c_str(), contLen);
}

void File::deserializeFile(std::ifstream& in) {
    size_t extLen;
    in.read(reinterpret_cast<char*>(&extLen), sizeof(extLen));
    extension.resize(extLen);
    in.read(&extension[0], extLen);

    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    size_t contLen;
    in.read(reinterpret_cast<char*>(&contLen), sizeof(contLen));
    content.resize(contLen);
    in.read(&content[0], contLen);
}