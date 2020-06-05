import random as rdm
import inspect
import re
import os

# Generació de dades de accelerometre
def imuData(file, array, varname, min, max, quantitat, decimals):
    file.write("const float " + varname + "[] = "+"{\n")
    for i in range(quantitat):
        if(i!=0):
            file.write(",\n")
        if(decimals==0):
            data = rdm.randint(-min, max)
            array.append(data)
        else:
            data = rdm.random()
            array.append(round((data*(max+min))-min, decimals))
        file.write(str(array[i]))
    file.write ("};\n\n")
    return array

# Generació de dades del motor
def motorData(file,angle, fase, nomAngle, nomFase, numDades):
    # Angle
    file.write("const uint16_t " + nomAngle + "[] =" + "{\n")
    for k in range(numDades):
        if(k!=0):
                file.write(",\n")
        for i in range(360):
            if(i!=0):
                file.write(",\n") 
            angle.append(i)
            if(i<120):
                fase.append(1)
            elif (i>=120 and i<=240):
                fase.append(2)
            elif (i>240):
                fase.append(3)
            file.write(str(angle[i]))
    file.write("};\n\n")
    # Fase (o bobina)
    file.write ("const uint16_t " + nomFase + "[] =" + "{\n")
    for j in range (len(fase)):
        if(j!=0):
            file.write(",\n")
        file.write(str(fase[j]))
    file.write("};\n\n")
    return angle, fase

# Creació d'arrays
# IMU
accelX = []
accelY = []
accelZ = []
gyroX = []
gyroY = []
gyroZ = []
magnetoX = []
magnetoY = []
magnetoZ = []

# Motor
angleMot = []
faseMot = []

# Creació del document
dir = os.path.dirname(__file__)
filename = os.path.join(dir, "IMULabs_proj\lib\mock.h")
f = open(filename,"w")
f.write("#ifndef LIB_MOCK_H_\n#define LIB_MOCK_H_\n\n#define IDLE 000\n#define READ 001\n#define SAVE 010\n#define W_R 011\n#define CHECK 100\n#define ERROR 111\n\n\n")
imuData(f, accelX, "accelX", 16, 16, 3600, 2)
imuData(f,accelY,"accelY",16, 16, 3600, 2)
imuData(f,accelZ, "accelZ",16, 16, 3600, 2)
imuData(f,gyroX,"gyroX", 2000, 2000, 3600, 2)
imuData(f,gyroY, "gyroY", 2000, 2000, 3600, 2)
imuData(f,gyroZ,"gyroZ", 2000, 2000, 3600, 2)
imuData(f,magnetoX, "magnetoX",4900, 4900, 3600, 2)
imuData(f,magnetoY, "magnetoY",4900, 4900, 3600, 2)
imuData(f,magnetoZ, "magnetoZ",4900, 4900, 3600, 2)
motorData(f, angleMot, faseMot, "angleMot", "faseMot",10)
f.write("#endif /* LIB_MOCK_H_ */")
f.close()
#print(len(accelX))

#mock = open("mock.h","r")
#print(mock.read())