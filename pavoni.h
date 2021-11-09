#ifndef __PAVONI_H__
#define __PAVONI_H__


#include <Arduino.h>

#define SLOPE 2.757905f
#define OFFSET -0.689476f

#define MARGIN 0.05f
#define ALPHA 0.2f
#define ADC_MAX 1024

#define VOLTAGE_BUFFER_SIZE 20

#define OVERPRESSURE 2.5f

enum error
{
    ERROR_NONE = 0,
    ERROR_PRESSURE_SENSOR,
    ERROR_OVERPRESSURE
};

class Pavoni
{
private:
    int m_turnedOn;
    int m_heating;

    int m_adcPin;
    int m_relayPin;

    float m_voltage[VOLTAGE_BUFFER_SIZE];
    int m_currentPos;

    float m_pressure;
    float m_maxPressure;

    error m_error;

    void UpdatePressure();

public:
    Pavoni() {}
    ~Pavoni() {}

    void Init(int adcPin, int relayPin, float maxPressure);
    void Update();

    void On();
    void Off();

    void SetMaxPressure(float maxPressure);

    float GetPressure() const;
    int IsTurnedOn() const;
    int IsHeating() const;

    error GetError() const;
};


#endif /* __PAVONI_H__ */
