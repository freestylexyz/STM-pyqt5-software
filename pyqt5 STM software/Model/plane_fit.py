# -*- coding: utf-8 -*-
"""
@Date     : 2021/2/3 07:05:18
@Author   : milier00
@FileName : plane_fit.py
"""
import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
from matplotlib import cm
from PIL import Image, ImageDraw
from numpy import genfromtxt
import pandas as pd
from scipy import optimize


raw_img = cv.imread('../data/real_stm_img.jpeg')
img = cv.cvtColor(raw_img, cv.COLOR_BGR2GRAY)

cv.imshow('img', img)
cv.waitKey()
cv.destroyAllWindows()



x = np.array([[i + 1] * img.shape[0] for i in range(img.shape[0])]).flatten()
y = np.array(list(np.arange(img.shape[1]) + 1) * img.shape[1])
z = img.flatten()
index = np.argmin(z)

# Create coefficient matrix A
A = np.zeros((3, 3))
for i in range(0, img.size):
    A[0, 0] = A[0, 0] + x[i] ** 2
    A[0, 1] = A[0, 1] + x[i] * y[i]
    A[0, 2] = A[0, 2] + x[i]
    A[1, 0] = A[0, 1]
    A[1, 1] = A[1, 1] + y[i] ** 2
    A[1, 2] = A[1, 2] + y[i]
    A[2, 0] = A[0, 2]
    A[2, 1] = A[1, 2]
    A[2, 2] = img.size

# Create B
b = np.zeros((3, 1))
for i in range(0, img.size):
    b[0, 0] = b[0, 0] + x[i] * z[i]
    b[1, 0] = b[1, 0] + y[i] * z[i]
    b[2, 0] = b[2, 0] + z[i]
# print(b)

# Solving x
A_inv = np.linalg.inv(A)
X = np.dot(A_inv, b)
# print('PlaneFit: The result of plane fitting is：z = %.3f * x + %.3f * y + %.3f' % (X[0, 0], X[1, 0], X[2, 0]))

# Calculate variance
R = 0
for i in range(0, img.size):
    R = R + (X[0, 0] * x[i] + X[1, 0] * y[i] + X[2, 0] - z[i]) ** 2
# print('PlaneFit: The variance is：%.*f' % (3, R))

# Display image
fig1 = plt.figure()
ax1 = fig1.add_subplot(111, projection='3d')
ax1.set_xlabel("x")
ax1.set_ylabel("y")
ax1.set_zlabel("z")
ax1.scatter(x, y, z, c='r', marker='.', alpha=0.03)
x_p = np.linspace(1, img.shape[0] + 1, 100)
y_p = np.linspace(1, img.shape[1] + 1, 100)
x_p, y_p = np.meshgrid(x_p, y_p)
# X[2, 0] = z[index] - X[0, 0] * x[index] - X[1, 0] * y[index]
z_p = X[0, 0] * x_p + X[1, 0] * y_p + X[2, 0]
new_z = X[0, 0] * x + X[1, 0] * y + X[2, 0]
new_z = z - new_z
new_z = np.reshape(new_z, (img.shape[0], img.shape[1])).astype(np.float32)

# ax1.plot_wireframe(x_p, y_p, z_p, rstride=10, cstride=10, alpha=0.7)
# ax1.scatter(x, y, new_z, c='g', marker='.', alpha=0.03)



img = cv.cvtColor(new_z, cv.COLOR_GRAY2BGR)
cv.imshow('img', img)
cv.waitKey()
cv.destroyAllWindows()

'''-----------------------'''
def planeerrors(para, points):
    """平面误差"""
    a0, a1, a2 = para
    return a0 * points[:, 0] + a1 * points[:, 1] + a2 - points[:, 2]
points = np.zeros((16129, 3))
points[:, 0]= x
points[:, 1]= y
# points[:, 2]= new_z.flatten()
points[:, 2]= z.flatten()

# tparap = optimize.leastsq(planeerrors, [X[0, 0], X[1, 0], X[2, 0]], points)
tparap = optimize.leastsq(planeerrors, [1,1,0], points)
para = tparap[0]

new_new_z = para[0]*x+para[1]*y+para[2]
# new_new_z = new_z.flatten() - new_new_z
new_new_z = z.flatten() - new_new_z
new_new_z = np.reshape(new_new_z, (img.shape[0], img.shape[1])).astype(np.float32)

z_p = para[0]*x_p + para[1]*y_p + para[2]

ax1.plot_wireframe(x_p, y_p, z_p, rstride=10, cstride=10, alpha=0.7)
ax1.scatter(x, y, new_new_z, c='b', marker='.', alpha=0.03)

print(new_new_z.shape)
plt.show()

img = cv.cvtColor(new_new_z, cv.COLOR_GRAY2BGR)
cv.imshow('img', new_new_z)
cv.waitKey()
cv.destroyAllWindows()

plt.imshow(new_new_z, cmap='Greys')
plt.show()

import cv2
import numpy as np
import matplotlib.pyplot as plt

# histogram normalization
def hist_normalization(img, a=0, b=255):
    # get max and min
    c = img.min()
    d = img.max()

    out = img.copy()

    # normalization
    out = (b-a) / (d - c) * (out - c) + a
    out[out < a] = a
    out[out > b] = b
    out = out.astype(np.uint8)

    return out

# Read image
# img = cv2.imread("../head_g_n.jpg",0).astype(np.float)
# histogram normalization
out = hist_normalization(img)

# # Display histogram
# plt.hist(out.ravel(), bins=255, rwidth=0.8, range=(0, 255))
# plt.savefig("out_his.jpg")
# plt.show()
#
# # Save result
# cv2.imshow("result", out)
# cv2.imwrite("out.jpg", out)
# cv2.waitKey(0)
# cv2.destroyAllWindows()