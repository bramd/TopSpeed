/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#include <Common/If/File.h>
#include <Common/If/Algorithm.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>

// Portable alternatives for Windows-specific functions
#ifndef _WIN32
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// sprintf_s replacement - just use snprintf
#define sprintf_s(buf, ...) snprintf(buf, sizeof(buf), __VA_ARGS__)

// fscanf_s replacement - just use fscanf (the _s versions add buffer sizes for strings)
#define fscanf_s fscanf

// strncpy_s replacement
inline void strncpy_s(char* dest, const char* src, size_t count) {
    strncpy(dest, src, count);
    if (count > 0) dest[count - 1] = '\0';
}
#endif

File::File(const Char filename[], UInt flags) :
    m_file(0),
    m_opened(false)
{
    if (flags & append)
    {
        if ((flags & read) && (flags & write))
        {
            if ((m_file = fopen( filename, "a+" )) == 0 )
            {
                COMMON("(!) Error opening file %s", filename);
            }
            else
            {
                m_opened = true;
            }
        }
        else if (flags & read)
        {
            if ((m_file = fopen( filename, "r" )) == 0 )
            {
                COMMON("(!) Error opening file %s", filename);
            }
            else
            {
                m_opened = true;
            }
        }
    }
    else
    {
        if (flags & write)
        {
            if (flags & create)
            {
                if ((m_file = fopen( filename, "w+" )) == 0 )
                {
                    COMMON("(!) Error opening file %s", filename);
                }
                else
                {
                    m_opened = true;
                }
            }
            else
            {
                if ((m_file = fopen( filename, "r+" )) == 0 )
                {
                    COMMON("(!) Error opening file %s", filename);
                }
                else
                {
                    m_opened = true;
                }
            }
        }
        else if (flags & read)
        {
            if ((m_file = fopen( filename, "r" )) == 0 )
            {
                COMMON("(!) Error opening file %s", filename);
            }
            else
            {
                m_opened = true;
            }
        }
    }
}       


File::~File( )
{
    if (m_file)
        fclose(m_file);
}



void File::rewind( )
{
    fseek(m_file, 0L, SEEK_SET );
}


void File::writeInt(Int i)
{
    char buffer[512];
    sprintf_s(buffer, "%i\n", i);
    fprintf(m_file, "%s", buffer);
}



Int File::readInt( )
{
    Int i;
    if (fscanf_s(m_file, "%i\n", &i) != 1)
        return -1;
    else 
        return i;
}


void File::writeFloat(Float f)
{
    char buffer[512];
    sprintf_s(buffer, "%f\n", f);
    fprintf(m_file, "%s", buffer);
}



void File::writeString(Char* string)
{
    fprintf(m_file, "%s", string);
}



Float File::readFloat( )
{
    Float f;
    fscanf_s(m_file, "%f\n", &f);
    return f;
}


void File::writeChar(Char c)
{
    char buffer[3];
    sprintf_s(buffer, "%c", c);
    fprintf(m_file, "%s", buffer);
}


Char File::readChar( )
{
    Char c;
    if (fscanf_s(m_file, "%c", &c) == EOF)
        return 0;
    else
        return c;
}


Boolean File::readInt(Char* field, Int& val, Int def)
{
    // Set to default
    val = def;
    // Go to the beginning of the file
    rewind( );
    Char buffer[128];
    while (fgets(buffer, 128, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // OK, we found the value!
                    val = atoi(equalsign+1);
                    return true;
                }
        }
    }
    // If arrived here, we didn't find the value
    return false;
}


Boolean File::readFloat(Char* field, Float& val, Float def)
{
   // Set to default
    val = def;
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    while (fgets(buffer, 128, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // OK, we found the value!
                    val = (Float) atof(equalsign+1);
                    return true;
                }
        }
    }
    // If arrived here, we didn't find the value
    return false;
}



Boolean File::readString(Char* field, Char* val, UInt size, Char* def)
{
   // Set to default
    strcpy(val, def);
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // replace newlines (or '\10') with '\0'
                    for (UInt i = 0; i < strlen(equalsign+1); ++i)
                        if (*(equalsign+i+1) == 10)
                            *(equalsign+i+1) = '\0';
                    // OK, we found the value!
                    UInt filenamesize = strlen(equalsign+1);
                    strncpy(val, equalsign+1, minimum<UInt>(size, filenamesize));
                    val[minimum<UInt>(size, filenamesize)] = '\0';
                    return true;
                }
        }
    }
    // If arrived here, we didn't find the value
    return false;
}

Boolean 
File::readKeyInt(Char* field, Int& val, Int def)
{
    // Set to default
    val = def;
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // replace newlines (or '\10') with '\0'
                    for (UInt i = 0; i < strlen(equalsign+1); ++i)
                        if (*(equalsign+i+1) == 10)
                            *(equalsign+i+1) = '\0';
                    // OK, we found the value!
                    Char strVal[MAX_PATH];
                    UInt filenamesize = strlen(equalsign+1);
                    strncpy_s(strVal, equalsign+1, minimum<UInt>(MAX_PATH, filenamesize));
                    strVal[minimum<UInt>(MAX_PATH, filenamesize)] = '\0';
                    val = ::atoi(strVal);
                    return true;
                }
        }
    }
    // If arrived here, we didn't find the value
    return false;
}


Boolean 
File::writeKeyInt(Char* field, Int val)
{
    // start a temporary file
    std::string tempBuffer;
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    Boolean fieldFound = false;
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        Boolean fieldNow = false;
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // replace newlines (or '\10') with '\0'
                    for (UInt i = 0; i < strlen(equalsign+1); ++i)
                        if (*(equalsign+i+1) == 10)
                            *(equalsign+i+1) = '\0';
                    // OK, we found the value!
                    Char line[MAX_PATH];
                    sprintf_s(line, "%s=%d\n", field, val);
                    fieldFound = true;
                    fieldNow = true;
                    tempBuffer.append(line);
                }
        }
        if (!fieldNow)
        {
            if (equalsign)
                *equalsign = '=';
            tempBuffer.append(buffer);
        }            
    }

    if (!fieldFound)
    {
        Char line[MAX_PATH];
        sprintf_s(line, "%s=%d\n", field, val);
        tempBuffer.append(line);
    }
    rewind( );
    writeString((char*) tempBuffer.c_str( ));

    return fieldFound;
}

Boolean 
File::writeKeyString(Char* field, Char* val)
{
    // start a temporary file
    std::string tempBuffer;
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    Boolean fieldFound = false;
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        Boolean fieldNow = false;
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // replace newlines (or '\10') with '\0'
                    for (UInt i = 0; i < strlen(equalsign+1); ++i)
                        if (*(equalsign+i+1) == 10)
                            *(equalsign+i+1) = '\0';
                    // OK, we found the value!
                    Char line[MAX_PATH];
                    sprintf_s(line, "%s=%s\n", field, val);
                    fieldFound = true;
                    fieldNow = true;
                    tempBuffer.append(line);
                }
        }
        if (!fieldNow)
        {
            if (equalsign)
                *equalsign = '=';
            tempBuffer.append(buffer);
        }            
    }

    if (!fieldFound)
    {
        Char line[MAX_PATH];
        sprintf_s(line, "%s=%s\n", field, val);
        tempBuffer.append(line);
    }
    rewind( );
    writeString((char*) tempBuffer.c_str( ));
    return fieldFound;
/*

    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) == strlen(field))
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    // replace newlines (or '\10') with '\0'
                    for (UInt i = 0; i < strlen(equalsign+1); ++i)
                        if (*(equalsign+i+1) == 10)
                            *(equalsign+i+1) = '\0';
                    // OK, we found the value!
                    UInt valsize = strlen(equalsign+1);
                    strncpy(val, equalsign+1, minimum<UInt>(size, filenamesize));
                    val[minimum<UInt>(size, filenamesize)] = '\0';
                    return true;
                }
        }
    }
    // If arrived here, we didn't find the value
    int lineLength = strlen(field) + strlen(val) + 3;
    Char* lineToWrite = new Char[lineLength];
    ::sprintf(lineToWrite, "%s=%s\n", field, val);
    writeString(lineToWrite);
    delete[] lineToWrite;
    return false;
    */
}


Boolean 
File::readVarString(Char* field, Char* val, UInt sizeVal, Char* var, UInt sizeVar, UInt count, Char* def)
{
    UInt n = 0;
    // Set to default
    strcpy(val, def);
    // Go to the beginning of the file
    rewind( );
    Char buffer[MAX_PATH];
    while (fgets(buffer, MAX_PATH, m_file) != 0)
    {
        // Search the line containing the field description
        Char* equalsign = 0;
        equalsign = strchr(buffer, '=');
        if (equalsign)
        {
            *equalsign = '\0';
            if (strlen(buffer) >= strlen(field))
            {
                if (strncmp(buffer, field, strlen(field)) == 0)
                {
                    if (n == count)
                    {
                        // fill in the varpart
                        strncpy(var, buffer+strlen(field), minimum<UInt>(equalsign - (buffer+strlen(field)), sizeVar));
                        var[minimum<UInt>(equalsign - (buffer+strlen(field)), sizeVar)] = '\0';
    
                        // replace newlines (or '\10') with '\0'
                        for (UInt i = 0; i < strlen(equalsign+1); ++i)
                            if (*(equalsign+i+1) == 10)
                                *(equalsign+i+1) = '\0';
                        // OK, we found the value!
                        UInt filenamesize = strlen(equalsign+1);
                        strncpy(val, equalsign+1, minimum<UInt>(sizeVal, filenamesize));
                        val[minimum<UInt>(sizeVal, filenamesize)] = '\0';
                        return true;
                    }
                    n++;
                }
            }
        }
    }
    // If arrived here, we didn't find the value
    return false;
}
