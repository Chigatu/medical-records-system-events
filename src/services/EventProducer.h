#pragma once
#include <string>
#include <memory>
#include <userver/rabbitmq/client.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace medical::infrastructure {

class EventProducer {
public:
    explicit EventProducer(userver::rabbitmq::ClientPtr rabbit_client);
    
    void PublishUserRegistered(int userId, const std::string& login, const std::string& email);
    void PublishUserLoggedIn(int userId, const std::string& login, const std::string& ip);
    void PublishPatientCreated(int patientId, const std::string& fullName, const std::string& snils);
    void PublishMedicalRecordCreated(int recordId, const std::string& code, int patientId, int doctorId);
    void PublishDiagnosisAdded(int recordId, int patientId, const std::string& code, const std::string& desc);

private:
    void Publish(const std::string& routingKey, const userver::formats::json::Value& payload);
    userver::rabbitmq::ClientPtr rabbit_;
};

}  // namespace medical::infrastructure
