# 🎬 MovieBase - Structured Data for Serials

Desktop-приложение на **C++20 + Qt 6**, предназначенное для работы с базой сериалов:

* импорт из CSV
* поиск и фильтрация
* сортировка
* просмотр подробной информации

---

## 🚀 Возможности

### 📥 Импорт данных

* Загрузка данных из CSV:

  * `movies.csv`
  * `persons.csv`
  * `casting.csv`
  
* Асинхронный импорт (через `QtConcurrent`)
* Прогресс-бар

### 🔍 Поиск и фильтрация

* Поиск по:

  * названию фильма
  * актёрам
  * ролям

* Фильтры:

  * год (min/max)
  * длительность (min/max)

### ↕️ Сортировка

* По:

  * названию
  * году
  * длительности

* Порядок:

  * Asc / Desc

### 📊 UI

* Таблица фильмов
* Детальная информация:

  * название
  * год
  * длительность
  * слоган
  * актёры и роли
  * постер

---

## 🏗️ Архитектура

Проект разделён на слои:

```
core/        → модели (Movie, Casting)
data/        → работа с SQLite (Repository)
service/     → бизнес-логика
importer/    → CSV импорт
ui/          → Qt интерфейс (MVC)
app/         → точка входа
```

### 📦 Слои

* **core**
  Чистые C++ структуры без зависимостей

* **data (Repository)**
  Работа с SQLite через Qt SQL
  CRUD + поиск

* **service**
  Бизнес-логика:

  * search
  * filter
  * sort

* **importer**

  * парсинг CSV
  * транзакционная загрузка в БД

* **ui**

  * `QMainWindow`
  * `QTableView` + кастомная модель

---

## 🗄️ База данных

SQLite (создаётся автоматически)

### Таблицы:

**movies**

```
id | title | year | length | slogan | image
```

**persons**

```
id | name
```

**casting**

```
movie_id | person_id | role
```

---

## 📁 Формат CSV

### movies.csv

```
id,title,year,length,...,slogan,image
```

### persons.csv

```
id,name
```

### casting.csv

```
movie_id,person_id,role
```

---

## 🧠 Особенности реализации

### ✔ Repository

* Инкапсуляция SQLite
* Поддержка транзакций
* JOIN для актёров

### ✔ Service Layer

* Комбинированный search + filter
* Использование `std::ranges`

### ✔ CSV Parser

* Поддержка кавычек `"..."`
* Обработка escaped `""`

### ✔ UI

* MVC (`QAbstractTableModel`)
* Асинхронные операции
* Lazy загрузка деталей фильма

---

## ⚙️ Сборка

### Требования

* CMake ≥ 3.21
* Qt 6 (Widgets, Sql, Concurrent)
* Компилятор с поддержкой C++20

---

### 🔧 Сборка (Windows / MinGW)

```bash
cmake -S . -B build
cmake --build build
```

Если Qt не найден:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
```

---

## ▶️ Запуск

```bash
cd build
./MovieApp.exe
```

---

## 📦 Деплой (Windows)

Автоматически вызывается после сборки:

```
windeployqt
```

**MovieBase** - Твоя личная база сериалов. 🎬
