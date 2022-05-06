from epics import PV,caget,caput
a=PV("testAPD:scope1:PressureValue0_RBV")
b=PV("testAPD:scope1:CalcPressure0.B")
print(a.value)
print(b.value)
caput("testAPD:scope1:CalcPressure0.B",a.value,wait=True)
print(b.value)
