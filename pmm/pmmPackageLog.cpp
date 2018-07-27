/* pmmPackageLog.cpp
 * Defines the Package Log (MLOG) and the Package Log Information (MLIN).
 *
 * By Henrique Bruno Fantauzzi de Almeida (aka SrBrahma) - Minerva Rockets, UFRJ, Rio de Janeiro - Brazil */

#define PMM_PACKAGE_LOG_DATA_INDEX 2
// 0 is MLOG
// 1 is MLIN String CRC
// 2 is data (PMM_PACKAGE_LOG_DATA_INDEX)
#include <crc16.h>
#include <pmmPackageLog.h>
#include <pmmConsts.h>



PmmPackageLog::PmmPackageLog()
{
    mPackageSizeInBytes = 0;
    mActualNumberVariables = 0;

    const PROGMEM char* packageLogHeader = "packageLogHeader"; // It isn't actually used. But will leave it for the future. (CMON 11 BYTES AT PROGMEM IS NOTHING)
    addCustomVariable(packageLogHeader, PMM_TELEMETRY_TYPE_UINT32, (void*)PMM_TELEMETRY_HEADER_LOG); // MLOG, the header. (void*) is to convert const* void to void*.

    const PROGMEM char* mlinStrincCrc = "mlinStrCrc"; // Same as the above funny commentary. Maybe you can't find it, it isn't funny at all.
    addCustomVariable(mlinStrincCrc, PMM_TELEMETRY_TYPE_UINT16, &mMlinStringCrc); // MLIN String CRC

}



void PmmPackageLog::updateMlinStringCrc()
{
    char buffer[512] = {0}; // No static needed, as it is called only once.
    unsigned variableIndex;
    for (variableIndex = PMM_PACKAGE_LOG_DATA_INDEX; variableIndex < mActualNumberVariables; variableIndex ++)
        strncat(buffer, mVariableNameArray[variableIndex], 512);

    //mMlinStringCrc = crc16(buffer, strlen(buffer));
}

uint8_t PmmPackageLog::variableTypeToVariableSize(uint8_t variableType)
{
    switch (variableType)
    {
        case PMM_TELEMETRY_TYPE_UINT8:
            return 1;
        case PMM_TELEMETRY_TYPE_INT8:
            return 1;
        case PMM_TELEMETRY_TYPE_UINT16:
            return 2;
        case PMM_TELEMETRY_TYPE_INT16:
            return 2;
        case PMM_TELEMETRY_TYPE_UINT32:
            return 4;
        case PMM_TELEMETRY_TYPE_INT32:
            return 4;
        case PMM_TELEMETRY_TYPE_FLOAT:
            return 4;
        case PMM_TELEMETRY_TYPE_UINT64:
            return 8;
        case PMM_TELEMETRY_TYPE_INT64:
            return 8;
        case PMM_TELEMETRY_TYPE_DOUBLE:
            return 8;
        default:    // Maybe will avoid internal crashes?
            PMM_DEBUG_PRINT("PmmPackage #1: Invalid variable type to size!");
            return 1;
    }
}

void PmmPackageLog::includeArrayInPackage(const char **variableName, uint8_t arrayType, void *arrayAddress, uint8_t arraySize)
{
    uint8_t counter;
    for (counter = 0; counter < arraySize; counter++)
        includeVariableInPackage(*variableName++, arrayType, (uint8_t*) arrayAddress + (variableTypeToVariableSize(arrayType) * counter));
}

void PmmPackageLog::includeVariableInPackage(const char *variableName, uint8_t variableType, void *variableAddress)
{
    uint8_t varSize = variableTypeToVariableSize(variableType);
    if (mActualNumberVariables >= PMM_TELEMETRY_LOG_NUMBER_VARIABLES)
    {
        #if PMM_DEBUG_SERIAL
            Serial.print("PmmPackage #1: Failed to add the variable \"");
            Serial.print(variableName);
            Serial.print("\". Exceeds the maximum number of variables in the Package Log.\n");
        #endif
        return;
    }
    if ((mPackageSizeInBytes + varSize) >= PMM_TELEMETRY_MAX_PAYLOAD_LENGTH)
    {
        #if PMM_DEBUG_SERIAL
            Serial.print("PmmPackage #2: Failed to add the variable \"");
            Serial.print(variableName);
            Serial.print("\". Exceeds the maximum payload length (tried to be ");
            Serial.print((mPackageSizeInBytes + varSize));
            Serial.print(", maximum is ");
            Serial.print(PMM_TELEMETRY_MAX_PAYLOAD_LENGTH);
            Serial.print(".\n");
        #endif
        return;
    }

    mVariableNameArray[mActualNumberVariables] = variableName;
    mVariableTypeArray[mActualNumberVariables] = variableType;
    mVariableSizeArray[mActualNumberVariables] = varSize;
    mVariableAddressArray[mActualNumberVariables] = (uint8_t*) variableAddress;
    mActualNumberVariables ++;
    mPackageSizeInBytes += varSize;

    if (mActualNumberVariables > PMM_PACKAGE_LOG_DATA_INDEX) // yeah it's right. It isn't actually necessary, just skip a few useless function calls.
        updateMlinStringCrc(); // Updates the Mlin string CRC.
}

void PmmPackageLog::addPackageBasicInfo(uint32_t* packageIdPtr, uint32_t* packageTimeMsPtr)
{
    const PROGMEM char* packageIdString = "packageId";
    const PROGMEM char* packageTimeString = "packageTime(ms)";
    includeVariableInPackage(packageIdString, PMM_TELEMETRY_TYPE_UINT32, packageIdPtr);
    includeVariableInPackage(packageTimeString, PMM_TELEMETRY_TYPE_UINT32, packageTimeMsPtr);
}


void PmmPackageLog::addMagnetometer(void* array)
{
    const PROGMEM char* arrayString[3] = {"magnetometerX(uT)", "magnetometerY(uT)", "magnetometerZ(uT)"};
    includeArrayInPackage(arrayString, PMM_TELEMETRY_TYPE_FLOAT, array, 3);
}

void PmmPackageLog::addGyroscope(void* array)
{
    const PROGMEM char* arrayString[3] = {"gyroscopeX(degree/s)", "gyroscopeY(degree/s)", "gyroscopeZ(degree/s)"};
    includeArrayInPackage(arrayString, PMM_TELEMETRY_TYPE_FLOAT, array, 3);
}

void PmmPackageLog::addAccelerometer(void* array)
{
    const PROGMEM char* arrayString[3] = {"accelerometerX(g)", "accelerometerY(g)", "accelerometerZ(g)"};
    includeArrayInPackage(arrayString, PMM_TELEMETRY_TYPE_FLOAT, array, 3);
}

void PmmPackageLog::addBarometer(void* barometer)
{
    const PROGMEM char* barometerPressureString = "barometerPressure(hPa)";
    includeVariableInPackage(barometerPressureString, PMM_TELEMETRY_TYPE_FLOAT, barometer);
}

void PmmPackageLog::addAltitudeBarometer(void* altitudePressure)
{
    const PROGMEM char* barometerAltitudeString = "barometerAltitude(m)";
    includeVariableInPackage(barometerAltitudeString, PMM_TELEMETRY_TYPE_FLOAT, altitudePressure);
}

void PmmPackageLog::addThermometer(void* thermometerPtr)
{
    const PROGMEM char* thermometerString = "temperature(C)";
    includeVariableInPackage(thermometerString, PMM_TELEMETRY_TYPE_FLOAT, thermometerPtr);
}

void PmmPackageLog::addImu(pmmImuStructType *pmmImuStructPtr)
{
    addAccelerometer(pmmImuStructPtr->accelerometerArray);
    addGyroscope(pmmImuStructPtr->gyroscopeArray);
    addMagnetometer(pmmImuStructPtr->magnetometerArray);

    addBarometer(&pmmImuStructPtr->pressure);
    addAltitudeBarometer(&pmmImuStructPtr->altitudePressure);
    addThermometer(&pmmImuStructPtr->temperature);
}

void PmmPackageLog::addGps(pmmGpsStructType* pmmGpsStruct)
{
    #ifdef GPS_FIX_LOCATION
        const PROGMEM char* gpsLatitudeString = "gpsLatitude";
        const PROGMEM char* gpsLongitudeString = "gpsLongitude";
        includeVariableInPackage(gpsLatitudeString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->latitude));
        includeVariableInPackage(gpsLongitudeString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->longitude));
    #endif

    #ifdef GPS_FIX_ALTITUDE
        const PROGMEM char* gpsAltitudeString = "gpsAltitude(m)";
        includeVariableInPackage(gpsAltitudeString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->altitude));
    #endif

    #ifdef GPS_FIX_SATELLITES
        const PROGMEM char* gpsSatellitesString = "gpsSatellites";
        includeVariableInPackage(gpsSatellitesString, PMM_TELEMETRY_TYPE_UINT8, &(pmmGpsStruct->satellites));
    #endif
    /*
    #ifdef GPS_FIX_SPEED
        const PROGMEM char* gpsHorizontalSpeedString = "gpsHorSpeed(m/s)";
        const PROGMEM char* gpsNorthSpeedString = "gpsNorthSpeed(m/s)";
        const PROGMEM char* gpsEastSpeedString = "gpsEastSpeed(m/s)";
        const PROGMEM char* gpsHeadingDegreeString = "gpsHeadingDegree";
        includeVariableInPackage(gpsHorizontalSpeedString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->horizontalSpeed));
        includeVariableInPackage(gpsNorthSpeedString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->northSpeed));
        includeVariableInPackage(gpsEastSpeedString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->eastSpeed));
        includeVariableInPackage(gpsHeadingDegreeString, PMM_TELEMETRY_TYPE_FLOAT, &(pmmGpsStruct->headingDegree));

        #ifdef GPS_FIX_ALTITUDE
            const PROGMEM char* gpsUpSpeedString = "gpsSpeedUp(m/s)";
            includeVariableInPackage(gpsUpSpeedString, PMM_TELEMETRY_TYPE_FLOAT, &pmmGpsStruct->upSpeed);
        #endif
    #endif*/
}





void PmmPackageLog::addCustomVariable(const char* variableName, uint8_t variableType, void* variableAddress)
{
    includeVariableInPackage(variableName, variableType, variableAddress);
}

uint8_t PmmPackageLog::getNumberOfVariables()
{
    return mActualNumberVariables;
}

uint8_t PmmPackageLog::getPackageSizeInBytes()
{
    return mPackageSizeInBytes;
}

const char** PmmPackageLog::getVariableNameArray()  { return mVariableNameArray;}
uint8_t* PmmPackageLog::getVariableTypeArray()      { return mVariableTypeArray;}
uint8_t* PmmPackageLog::getVariableSizeArray()      { return mVariableSizeArray;}
uint8_t** PmmPackageLog::getVariableAddressArray()     { return mVariableAddressArray;}

void PmmPackageLog::updatePackageLogInfoRaw()
{
    uint8_t variableCounter;
    uint16_t stringLength;

    mPackageLogInfoRawArray[0] = mActualNumberVariables;
    mPackageLogInfoArrayLength = 1;

    /* Add the variable types */
    for (variableCounter = 0; variableCounter < mActualNumberVariables; variableCounter++)
    {
        if (variableCounter % 2) // If is odd (if rest is 1)
        {
            mPackageLogInfoRawArray[mPackageLogInfoArrayLength] |= mVariableTypeArray[variableCounter]; // Add it on the right
            mPackageLogInfoArrayLength++;
        }
        else // Is even (rest is 0). As it happens first than the odd option, no logical OR is needed.
            mPackageLogInfoRawArray[mPackageLogInfoArrayLength] = mVariableTypeArray[variableCounter] << 4; // Shift Left 4 positions to add to the left
    }
    if (variableCounter % 2) // If for loop ended on a even number (and now the variable is odd due to the final increment that made it >= mActualNumberVariables)
        mPackageLogInfoArrayLength++; // As this variable only increased in odd numbers.

    /* Now add the strings of each variable */
    for (variableCounter = 0; variableCounter < mActualNumberVariables; variableCounter++)
    {
        stringLength = strlen(mVariableNameArray[variableCounter]) + 1; // + 1 Adds the \0 char.
        memcpy(mPackageLogInfoRawArray + mPackageLogInfoArrayLength, mVariableNameArray[variableCounter], stringLength);
        mPackageLogInfoArrayLength += stringLength;
    }
    mPackageLogInfoNumberOfPackets = ceil(mPackageLogInfoArrayLength / (float) PMM_TELEMETRY_PACKAGE_LOG_INFO_MAX_PAYLOAD_LENGTH);
}

uint8_t PmmPackageLog::getPackageLogInfoInTelemetryFormat(uint8_t* arrayToCopyTo, uint8_t requestedPacket)
{
    // Format is ["MLIN"][CRC of the actual packet: 2B][Packet X of Y - 1: 1B] | [Number variables: 1B][Variable types: 4b][Variables strings]
    //
    // Header for all packets:
    // arrayToCopy[0~3] Package Header
    // arrayToCopy[4~5] CRC of the packet
    // arrayToCopy[6] Packet X
    // arrayToCopy[7] of a total of (Y - 1)

    uint16_t packetLength = PMM_TELEMETRY_PACKAGE_LOG_INFO_HEADER_LENGTH; // The Package Header default length.
    uint8_t totalNumberOfPackets
    uint16_t crc16Var;
    uint16_t payloadBytesInThisPacket;

    if (requestedPacket >= mPackageLogInfoNumberOfPackets) // If the requested ID is invalid.
        return 0;

    payloadBytesInThisPacket = mPackageLogInfoArrayLength - (requestedPacket * PMM_TELEMETRY_PACKAGE_LOG_INFO_MAX_PAYLOAD_LENGTH);
    if (payloadBytesInThisPacket >
    memcpy(arrayToCopyTo, (void*) PMM_TELEMETRY_HEADER_LOG_INFO, 4); // Adds the Package Header.

    // 4~5 are the crc16, it's added on the next lines!

    arrayToCopyTo[6] = requestedPacket;
    arrayToCopyTo[7] = mPackageLogInfoNumberOfPackets - 1;

    memcpy(arrayToCopyTo, (void*) PMM_TELEMETRY_HEADER_LOG_INFO, packetLength);

    crc16Var = crc16(arrayToCopyTo+6, strlen(mStrings[requestedStringId]) + 2, crc16(arrayToCopyTo, 4));
    // It first does the CRC16 of the Package Header (length 4),
    // Then skips the Header and these 2 bytes destined to the CRC16 and do the CRC of the rest (strlen(string) + (String X of Y ( = 2)).

    arrayToCopyTo[4] = crc16Var & 0x0F; // Little endian! First the Least Significant Byte!
    arrayToCopyTo[5] = crc16Var & 0xF0; // Little endian! Then the Most Significant Byte!

    return packageLength;
}


#if PMM_DEBUG_SERIAL
// Note for the 2 functions below:
// There are faster ways to print the debugs, but since it isn't something that is going to be used frequently,
// I (HB :) ) will spend my precious time on other stuffs)

void PmmPackageLog::debugPrintLogHeader()
{
    unsigned variableIndex;
    char buffer[512] = {0}; // No static needed, as it is called only once.

    if (mActualNumberVariables > PMM_PACKAGE_LOG_DATA_INDEX)
        snprintf(buffer, 512, "%s", mVariableNameArray[PMM_PACKAGE_LOG_DATA_INDEX]);

    for (variableIndex = PMM_PACKAGE_LOG_DATA_INDEX + 1; variableIndex < mActualNumberVariables; variableIndex ++)
    {
        snprintf(buffer, 512, "%s | %s", buffer, mVariableNameArray[variableIndex]);
    }
    Serial.println(buffer);
}



void PmmPackageLog::debugPrintLogContent()
{
    unsigned variableIndex;
    static char buffer[512]; // Static for optimization
    buffer[0] = '\0';
    for (variableIndex = 0; variableIndex < mActualNumberVariables; variableIndex ++)
    {
        switch(mVariableTypeArray[variableIndex])
        {
            case PMM_TELEMETRY_TYPE_FLOAT: // first as it is more common
                snprintf(buffer, 512, "%s%f, ", buffer, *(float*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_UINT32:
                snprintf(buffer, 512, "%s%lu, ", buffer, *(uint32_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_INT32:
                snprintf(buffer, 512, "%s%li, ", buffer, *(int32_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_UINT8:
                snprintf(buffer, 512, "%s%u, ", buffer, *(uint8_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_INT8:
                snprintf(buffer, 512, "%s%i, ", buffer, *(int8_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_UINT16:
                snprintf(buffer, 512, "%s%u, ", buffer, *(uint16_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_INT16:
                snprintf(buffer, 512, "%s%i, ", buffer, *(int16_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_UINT64:
                snprintf(buffer, 512, "%s%llu, ", buffer, *(uint64_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_INT64:
                snprintf(buffer, 512, "%s%lli, ", buffer, *(int64_t*) (mVariableAddressArray[variableIndex]));
                break;
            case PMM_TELEMETRY_TYPE_DOUBLE:
                snprintf(buffer, 512, "%s%f, ", buffer, *(double*) (mVariableAddressArray[variableIndex]));
                break;
            default:    // If none above,
                snprintf(buffer, 512, "%s%s, ", buffer, ">TYPE ERROR HERE!<");
                break;
        } // switch end
    } // for loop end
    Serial.println(buffer);
} // end of function debugPrintLogContent()


#endif
