#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstring>

class aes256_cbc {
private:
    std::vector<uint8_t> iv;
public:
    explicit aes256_cbc(std::vector<uint8_t> iv);
    void encrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const;
    void decrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const;
};

namespace security {
	void init();
	std::string encrypt(const std::string& text);
	std::string decrypt(const std::string& text);
};