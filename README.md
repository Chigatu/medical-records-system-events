# Medical Records Management System (Event-Driven Architecture)

REST API сервис с Event-Driven архитектурой, CQRS и RabbitMQ (Вариант 20, ДЗ №6).

## Функциональность

- Регистрация и аутентификация пользователей (JWT)
- Поиск пользователей по маске имени
- Регистрация пациентов
- Поиск пациентов по ФИО
- Создание медицинских записей
- Получение истории записей пациента
- **Event-Driven архитектура** — все изменения публикуются в RabbitMQ
- **CQRS** — разделение на read/write модели (PostgreSQL + MongoDB)
- Кеширование с Redis
- Rate limiting (Token Bucket)

## Технологии

- C++20 + **userver** (асинхронный фреймворк Яндекса)
- **RabbitMQ 3** — брокер сообщений (Event-Driven)
- **PostgreSQL 15** — write-модель (CQRS)
- **MongoDB 7** — read-модель (CQRS)
- **Redis 7** — кеширование + rate limiting
- JWT аутентификация
- Docker (docker-compose)

## События в системе

| Событие | Routing Key | Producer | Consumers |
|---------|-------------|----------|-----------|
| `UserRegistered` | `user.registered` | AuthService | AuditService, NotificationService |
| `UserLoggedIn` | `user.logged_in` | AuthService | AuditService |
| `PatientCreated` | `patient.created` | PatientService | AuditService, SearchIndexService |
| `MedicalRecordCreated` | `record.created` | RecordService | AuditService, StatisticsService, CacheInvalidationService |
| `DiagnosisAdded` | `diagnosis.added` | RecordService | StatisticsService |

## CQRS

| Операция | Модель | База данных |
|----------|--------|-------------|
| Создание пациента | Write | PostgreSQL |
| Поиск пациентов | Read | MongoDB |
| Создание записи | Write | PostgreSQL |
| История записей | Read | MongoDB |

## Быстрый старт (Docker)

1. Установите Docker Desktop
2. Склонируйте репозиторий:
```
git clone https://github.com/Chigatu/medical-records-system-events.git
cd medical-records-system-events
```
3. Запустите все сервисы:
```
docker-compose up --build
```
4. Проверьте API:
```
curl http://localhost:8080/health
```
5. RabbitMQ Management UI:
```
http://localhost:15672
# Логин: medical_user, Пароль: medical_pass
```

## API Endpoints

| Метод | URL | Описание | Событие |
|-------|-----|----------|---------|
| GET | /health | Проверка работоспособности | — |
| POST | /api/auth/register | Регистрация | `UserRegistered` |
| POST | /api/auth/login | Вход | `UserLoggedIn` |
| POST | /api/patients | Создание пациента | `PatientCreated` |
| GET | /api/patients/search?fullName={name} | Поиск пациентов (Read) | — |
| POST | /api/medical-records | Создание записи | `MedicalRecordCreated`, `DiagnosisAdded` |
| GET | /api/medical-records/patient/{id} | История записей (Read) | — |

## Структура проекта

```
medical-records-system-events/
├── src/
│   ├── handlers/               # HTTP обработчики
│   ├── models/                 # Доменные модели
│   ├── repositories/           # Репозитории (PostgreSQL, MongoDB)
│   ├── services/               # Сервисы + EventProducer + EventConsumer
│   └── main.cpp                # Точка входа
├── events/
│   ├── event_catalog.md        # Каталог всех событий
│   ├── event_driven_design.md  # Описание Event-Driven архитектуры
│   └── cqrs_design.md          # CQRS проектирование
├── configs/                    # Конфигурация userver
├── database/                   # PostgreSQL + MongoDB скрипты
├── Dockerfile
├── docker-compose.yaml         # API + PostgreSQL + MongoDB + Redis + RabbitMQ
└── README.md
```

## Тестирование

```
# Запуск тестов API
./test_api_userver.sh

# Просмотр событий в RabbitMQ
open http://localhost:15672

# Просмотр логов consumer
docker logs medical-api | grep "Event"
```
