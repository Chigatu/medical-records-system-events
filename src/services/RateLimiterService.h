#pragma once
#include <string>
#include <chrono>
#include <memory>
#include "CacheService.h"

namespace medical::infrastructure {

class RateLimiterService {
public:
    RateLimiterService(std::shared_ptr<CacheService> cache, int max_tokens, std::chrono::seconds window);
    
    bool IsAllowed(const std::string& client_id);
    int GetRemaining(const std::string& client_id);
    int GetLimit() const { return max_tokens_; }
    std::chrono::seconds GetWindow() const { return window_; }
    
private:
    std::shared_ptr<CacheService> cache_;
    int max_tokens_;
    std::chrono::seconds window_;
};

}  // namespace medical::infrastructure
