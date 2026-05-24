#include "login_handler.hpp"
#include <userver/formats/json.hpp>
#include <userver/components/component_context.hpp>
#include "repositories/RepositoryFactory.h"
#include <ctime>

namespace medical_api {

LoginHandler::LoginHandler(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
    auto user_service = std::make_shared<medical::application::UserService>(
        RepositoryFactory::GetUserRepo());
    auth_service_ = std::make_shared<medical::application::AuthService>(
        user_service, RepositoryFactory::GetJwtService());
}

std::string LoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const {
    // Rate Limiting
    std::string client_ip = request.GetRemoteAddress();
    auto rate_limiter = RepositoryFactory::GetLoginRateLimiter();
    
    if (!rate_limiter->IsAllowed(client_ip)) {
        auto& response = request.GetHttpResponse();
        response.SetStatus(userver::server::http::HttpStatus::kTooManyRequests);
        response.SetHeader("X-RateLimit-Limit", std::to_string(rate_limiter->GetLimit()));
        response.SetHeader("X-RateLimit-Remaining", "0");
        response.SetHeader("X-RateLimit-Reset", std::to_string(std::time(nullptr) + 60));
        response.SetHeader("Retry-After", "60");
        return R"({"error":"Too many login attempts. Try again later."})";
    }
    
    auto json = userver::formats::json::FromString(request.RequestBody());
    
    medical::application::dto::LoginRequestDTO dto;
    dto.login = json["login"].As<std::string>();
    dto.password = json["password"].As<std::string>();
    
    auto login_response = auth_service_->login(dto);
    
    // Добавляем заголовки rate limit
    auto& response = request.GetHttpResponse();
    response.SetHeader("X-RateLimit-Limit", std::to_string(rate_limiter->GetLimit()));
    response.SetHeader("X-RateLimit-Remaining", std::to_string(rate_limiter->GetRemaining(client_ip)));
    response.SetHeader("X-RateLimit-Reset", std::to_string(std::time(nullptr) + 60));
    
    if (!login_response.has_value()) {
        response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
        return R"({"error":"Invalid credentials"})";
    }
    
    userver::formats::json::ValueBuilder resp;
    resp["token"] = login_response->token;
    
    userver::formats::json::ValueBuilder user;
    user["id"] = login_response->user.id;
    user["login"] = login_response->user.login;
    user["email"] = login_response->user.email;
    user["fullName"] = login_response->user.fullName;
    resp["user"] = user;
    
    return userver::formats::json::ToString(resp.ExtractValue());
}

}  // namespace medical_api
