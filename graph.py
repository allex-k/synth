import numpy as np
import matplotlib.pyplot as plt

def triangle( x):
    return np.arcsin(np.sin(x))*2. / np.pi; 
def saw( x):
    return np.arctan(np.tan(-x / 2. + np.pi / 2))*2. / np.pi; 
def square( x):
    return np.floor(np.sin(x))*2. + 1.;

def sawSmooth(x):
    output = 0.; dCustom = 12;
    
    for n in range(1, dCustom, 1):
        output += (np.sin(n*x)) / n;
    return output*2./np.pi
    
	#for (float n = 1.f; n < dCustom; n++)
	#	output += (std::sin(n*x)) / n;
	#return output * (2.0 / PI);

x = np.linspace(-np.pi, np.pi, 128)

plt.subplot(6,1,1)

plt.plot(x, np.sin(x));
plt.xlabel('sine')

plt.subplot(6,1,2)
plt.plot(x, triangle(x));

plt.subplot(6,1,3)
plt.plot(x, saw(x));

plt.subplot(6,1,4)
plt.plot(x, sawSmooth(x));

plt.subplot(6,1,5)
plt.plot(x, square(x));

plt.subplot(6,1,6)
plt.plot(x, np.random.random(x.size)*2-1);

plt.show();