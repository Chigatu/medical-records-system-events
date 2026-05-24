#include "RateLimiterService.h"

namespace medical::infrastructure {

RateLimiterService::RateLimiterService(std::shared_ptr<CacheService> cache, 
                                         int max_tokens, 
                                         std::chrono::seconds window)
    : cache_(std::move(cache))
    , max_tokens_(max_tokens)
    , window_(window) {}

bool RateLimiterService::IsAllowed(const std::string& client_id) {
    return cache_->CheckRateLimit(client_id, max_tokens_, window_);
}

int RateLimiterService::GetRemaining(const std::string& client_id) {
    return cache_->GetRemainingTokens(client_id, max_tokens_);
}

}  // namespace medical::infrastructure
