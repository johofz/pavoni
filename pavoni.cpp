#include "pavoni.h"


void Pavoni::Init(int adcPin, int relayPin, float maxPressure)
{
    m_adcPin = adcPin;
    m_relayPin = relayPin;
    m_maxPressure = maxPressure;

    m_turnedOn = 0;
    m_heating = 0;

    pinMode(m_relayPin, OUTPUT);
    digitalWrite(m_relayPin, LOW);

    float initialReading = (float)(analogRead(m_adcPin)) / 1024.0f * 2.0f;
    for (int i = 0; i < VOLTAGE_BUFFER_SIZE; i++)
    {
        m_voltage[i] = initialReading;
    }
    m_currentPos = 0;

    m_pressure = initialReading * SLOPE + OFFSET;
    UpdatePressure();
}

void Pavoni::Update()
{
    UpdatePressure();

    if (m_turnedOn)
    {
        if (m_heating)
        {
            if (m_pressure < m_maxPressure * (1.0f + MARGIN))
            {
                digitalWrite(m_relayPin, HIGH);
            }
            else
            {
                m_heating = 0;
                digitalWrite(m_relayPin, LOW);
            }
        }
        else    // Cooldown-Phase
        {
            if (m_pressure > m_maxPressure * (1.0f - MARGIN))
            {
                digitalWrite(m_relayPin, LOW);
            }
            else
            {
                m_heating = 1;
                digitalWrite(m_relayPin, HIGH);
            }
        }
    }
    else
    {
        digitalWrite(m_relayPin, LOW);
    }
}

void Pavoni::On()
{
    m_turnedOn = 1;
}

void Pavoni::Off()
{
    m_turnedOn = 0;
    digitalWrite(m_relayPin, LOW);
}

void Pavoni::UpdatePressure()
{
    m_voltage[m_currentPos++] = (float)(analogRead(m_adcPin)) / 1024.0f * 2.0f;
    m_currentPos = m_currentPos % VOLTAGE_BUFFER_SIZE;

    float avg = 0;
    for (int i = 0; i < VOLTAGE_BUFFER_SIZE; i++)
    {
        avg += m_voltage[i];
    }
    avg = avg / VOLTAGE_BUFFER_SIZE;


    float p = avg * SLOPE + OFFSET;
    m_pressure = (1.0f - ALPHA) * p + ALPHA * m_pressure;
}

void Pavoni::SetMaxPressure(float maxPressure)
{
    if (maxPressure >= 1.0f && maxPressure <= 2.0f)
    {
        m_maxPressure = maxPressure;
    }
}

float Pavoni::GetPressure() const
{
    return m_pressure;
}

int Pavoni::IsTurnedOn() const
{
    return m_turnedOn;
}

int Pavoni::IsHeating() const
{
    return m_heating;
}
