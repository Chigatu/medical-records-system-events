#pragma once
#include <string>
#include <optional>
#include <chrono>
#include <userver/storages/redis/client.hpp>
#include <userver/storages/redis/component.hpp>

namespace medical::infrastructure {

class CacheService {
public:
    explicit CacheService(userver::storages::redis::ClientPtr redis_client);
    
    // Кеширование данных
    void Set(const std::string& key, const std::string& value, std::chrono::seconds ttl);
    std::optional<std::string> Get(const std::string& key);
    
    // Инвалидация кеша
    void Invalidate(const std::string& key);
    void InvalidatePattern(const std::string& pattern);
    
    // Для rate limiting (Token Bucket)
    bool CheckRateLimit(const std::string& key, int max_tokens, std::chrono::seconds window);
    int GetRemainingTokens(const std::string& key, int max_tokens);

private:
    userver::storages::redis::ClientPtr redis_;
};

}  // namespace medical::infrastructure
