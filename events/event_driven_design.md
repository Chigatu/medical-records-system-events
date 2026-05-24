# Event-Driven Architecture Design

## 1. Анализ событий в системе

### Команды и события

| Команда (API) | Событие | Когда публикуется |
|---------------|---------|-------------------|
| `POST /api/auth/register` | `UserRegistered` | После успешной регистрации |
| `POST /api/auth/login` | `UserLoggedIn` | После успешной аутентификации |
| `POST /api/patients` | `PatientCreated` | После создания пациента |
| `POST /api/medical-records` | `MedicalRecordCreated` | После создания записи |
| `POST /api/medical-records` | `DiagnosisAdded` | При наличии диагноза в записи |

### Потребители событий

| Consumer | События | Действие |
|----------|---------|----------|
| **AuditService** | Все события | Логирование действий в БД аудита |
| **NotificationService** | `UserRegistered` | Отправка email новому врачу |
| **StatisticsService** | `MedicalRecordCreated`, `DiagnosisAdded` | Обновление статистики по диагнозам |
| **CacheInvalidationService** | `MedicalRecordCreated` | Сброс кеша `records:patient:{id}` |
| **SearchIndexService** | `PatientCreated` | Обновление поискового индекса |

---

## 2. Компоненты системы

### Event Producers (производители)

- **AuthService** — публикует `UserRegistered`, `UserLoggedIn`
- **PatientService** — публикует `PatientCreated`
- **RecordService** — публикует `MedicalRecordCreated`, `DiagnosisAdded`

### Event Consumers (потребители)

- **AuditService** — подписан на все события (`#`)
- **NotificationService** — подписан на `user.registered`
- **StatisticsService** — подписан на `record.*`, `diagnosis.*`
- **CacheInvalidationService** — подписан на `record.created`
- **SearchIndexService** — подписан на `patient.created`

---

## 3. Поток событий

```
┌──────────────┐     ┌──────────────┐     ┌─────────────────┐
│  REST API    │────▶│   Broker     │────▶│   Consumers     │
│  (Producer)  │     │  (RabbitMQ)  │     │                 │
├──────────────┤     ├──────────────┤     ├─────────────────┤
│ AuthService  │     │ medical.events│     │ AuditService    │
│ PatientSvc   │     │   (topic)    │     │ NotificationSvc │
│ RecordSvc    │     │              │     │ StatisticsSvc   │
└──────────────┘     └──────────────┘     │ CacheInvalidSvc │
                                          │ SearchIndexSvc  │
                                          └─────────────────┘
```

### Пример потока: Создание медицинской записи

1. Врач отправляет `POST /api/medical-records`
2. **RecordService** создает запись в БД
3. **RecordService** публикует `MedicalRecordCreated` в RabbitMQ
4. **AuditService** получает событие → логирует действие
5. **StatisticsService** получает событие → обновляет счетчик диагнозов
6. **CacheInvalidationService** получает событие → сбрасывает кеш пациента

---

## 4. Брокер сообщений: RabbitMQ

### Выбор RabbitMQ

**Почему RabbitMQ:**
- Проще в настройке, чем Kafka (меньше компонентов)
- Поддерживает topic exchanges (удобно для маршрутизации)
- Достаточно для учебного проекта с 5 событиями
- Есть готовый Docker-образ

### Формат сообщений

Все сообщения в формате **JSON** со стандартной оберткой:

```json
{
  "eventId": "uuid",
  "eventType": "UserRegistered",
  "timestamp": "ISO8601",
  "payload": { ... }
}
```

### Гарантии доставки

- **At-least-once** — базовый уровень для всех событий
- Подтверждение (ACK) после успешной обработки
- Повторная отправка при отсутствии ACK
- Идемпотентность через `eventId` (consumer проверяет, не обработано ли событие ранее)

### Exchange и маршрутизация

| Exchange | Тип | Routing Keys |
|----------|------|--------------|
| `medical.events` | topic | `user.*`, `patient.*`, `record.*`, `diagnosis.*` |

| Очередь | Binding Key | Consumer |
|---------|-------------|----------|
| `audit.queue` | `#` | AuditService |
| `notification.queue` | `user.registered` | NotificationService |
| `statistics.queue` | `record.*`, `diagnosis.*` | StatisticsService |
| `cache_invalidation.queue` | `record.created` | CacheInvalidationService |
| `search_index.queue` | `patient.created` | SearchIndexService |

---

## 5. Масштабирование

- **Multiple consumers** на одной очереди — RabbitMQ распределяет сообщения round-robin
- **Prefetch limit** — ограничение количества неподтвержденных сообщений
- **Dead Letter Exchange (DLX)** — для сообщений, которые не удалось обработать
- **Retry с backoff** — повторная обработка с увеличивающейся задержкой
