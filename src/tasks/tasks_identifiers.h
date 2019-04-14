#ifndef TASKS_IDENTIFIERS_H
#define TASKS_IDENTIFIERS_H

//! \brief Набор идентификаторов имеющихся задач.

enum class TASK_ID {
    LandingLinear,    ///< Спуска ЛА на планету (линеаризованный).
    LandingGauss,     ///< Спуска ЛА на планету (гауссовский).
    VanDerPolLinear,  ///< Осциллятор Ван-дер-Поля (линеаризованный).
    VanDerPolGauss,   ///< Осциллятор Ван-дер-Поля (гауссовский).
    ScalarLinear,     ///< Тестовый скалярный пример (линеаризованный).
    ScalarGauss,      ///< Тестовый скалярный пример (гауссовский).
    DVanDerPolLinear,   ///< Осциллятор Ван-дер-Поля дискретный (линеаризованный).
    DVanDerPolGauss,   ///< Осциллятор Ван-дер-Поля дискретный (гауссовский).
    LDScalarRejectionLinear,  ///< Тестовый скалярный логико-динамический пример с отказами (гауссовский).
    LDLandingRejection3DLinear, ///< Спуск ЛА 3-мерный логико-динамический с отказами (линеаризованный).
    LDLandingRejection6DLinear, ///< Спуск ЛА 6-мерный логико-динамический с отказами (линеаризованный).
    LDVanDerPolRejectionLinear, ///< Осциллятор Ван-дер-Поля логико-динамический с отказами (линеаризованный).
};

#endif // TASKS_IDENTIFIERS_H
