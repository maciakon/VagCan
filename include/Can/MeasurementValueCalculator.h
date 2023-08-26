#include "Can\Pids.h"

class MeasurementValueCalculator
{
public:
    humanReadablePidValue calculateValueFromSensorReading(byte *sensorReading);
};