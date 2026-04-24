#include "crypto.h"
#include <argon2.h>
#include <stdexcept>

std::string PasswordHasher::hash(const std::string &password)
{
    char hash[128];

    int result = argon2id_hash_encoded(
        2,              
        1 << 16,
        1, 
        password.c_str(),
        password.size(),
        "salt123", 
        12,
        32,
        hash,
        sizeof(hash)
    );

    if (result != ARGON2_OK)
    {
        throw std::runtime_error("Argon2 hashing failed");
    }

    return std::string(hash);
    
}