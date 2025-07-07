#ifndef PARALLELCONNECTIONSVALUES_H
#define PARALLELCONNECTIONSVALUES_H

// Values has been defined by the SDK
class ParallelConnectionsValues
{
public:
    static int getMinValue()
    {
        return 1;
    }

    static int getMaxValue()
    {
        return 100;
    }

    static int getDefaultValue()
    {
        return 8;
    }

    static bool contains(int value)
    {
        return value >= getMinValue() && value <= getMaxValue();
    }
};

#endif // PARALLELCONNECTIONSVALUES_H
