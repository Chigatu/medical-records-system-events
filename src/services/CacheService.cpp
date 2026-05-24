#include "CacheService.h"
#include <userver/storages/redis/reply.hpp>
#include <iostream>

namespace medical::infrastructure {

CacheService::CacheService(userver::storages::redis::ClientPtr redis_client)
    : redis_(std::move(redis_client)) {}

void CacheService::Set(const std::string& key, const std::string& value, std::chrono::seconds ttl) {
    try {
        redis_->Set(key, value, ttl).Get();
    } catch (const std::exception& e) {
        std::cerr << "Cache Set error: " << e.what() << std::endl;
    }
}

std::optional<std::string> CacheService::Get(const std::string& key) {
    try {
        auto result = redis_->Get(key).Get();
        if (result) return *result;
    } catch (const std::exception& e) {
        std::cerr << "Cache Get error: " << e.what() << std::endl;
    }
    return std::nullopt;
}

void CacheService::Invalidate(const std::string& key) {
    try {
        redis_->Del(key).Get();
    } catch (const std::exception& e) {
        std::cerr << "Cache Invalidate error: " << e.what() << std::endl;
    }
}

void CacheService::InvalidatePattern(const std::string& pattern) {
    try {
        auto keys = redis_->Keys(pattern).Get();
        if (!keys.empty()) {
            redis_->Del(keys).Get();
        }
    } catch (const std::exception& e) {
        std::cerr << "Cache InvalidatePattern error: " << e.what() << std::endl;
    }
}

bool CacheService::CheckRateLimit(const std::string& key, int max_tokens, std::chrono::seconds window) {
    try {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::string bucket_key = "ratelimit:" + key;
        
        // Используем Redis для Token Bucket
        redis_->Incr(bucket_key).Get();
        redis_->Expire(bucket_key, window).Get();
        
        auto count = redis_->Get(bucket_key).Get();
        if (count) {
            return std::stoi(*count) <= max_tokens;
        }
    } catch (const std::exception& e) {
        std::cerr << "RateLimit error: " << e.what() << std::endl;
        return true; // При ошибке пропускаем запрос
    }
    return true;
}

int CacheService::GetRemainingTokens(const std::string& key, int max_tokens) {
    try {
        std::string bucket_key = "ratelimit:" + key;
        auto count = redis_->Get(bucket_key).Get();
        if (count) {
            int used = std::stoi(*count);
            return std::max(0, max_tokens - used);
        }
    } catch (...) {}
    return max_tokens;
}

}  // namespace medical::infrastructure
