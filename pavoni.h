#ifndef __PAVONI_H__
#define __PAVONI_H__

#include <Arduino.h>

class Pavoni
{
private:
    int m_turnedOn;
    int m_heating;

    int m_adcPin;
    int m_relayPin;

    float m_pressure;
    float m_maxPressure;

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
};


#endif /* __PAVONI_H__ */
