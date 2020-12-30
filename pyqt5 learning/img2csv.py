# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/30 17:43:56
@Author   : milier00
@FileName : img2csv.py
"""
# #
# # color -> gray
# #
# import cv2 as cv
#
# #读取图片
# img=cv.imread('.\dataset\scan_example.png')
# #修改图片显示的大小
# img = cv.resize(img,dsize=(64,64))
# cv.imshow('read_img',img)
#
# #图片灰度转换
# gray_img = cv.cvtColor(img,cv.COLOR_BGR2GRAY)
# cv.imshow('gray_img',gray_img)
#
# #保存图片
# cv.imwrite('.\dataset\scan_example.jpg', gray_img)
#
#
# cv.waitKey(0)
# #关闭所有图片窗口
# cv.destroyAllWindows()


import csv
import os
import cv2

IMG_DIR = r"D:\MyCode\Pythonscripts\test\dataset"

def convert_img_to_csv(img_dir):
    with open(r'D:\MyCode\Pythonscripts\test\dataset\scan_example.csv', 'w', newline='') as f:
        # column_name = ['label']
        column_name = []
        column_name.extend('pixel%d' % i for i in range(432 * 432))

        writer = csv.writer(f)
        writer.writerow(column_name)

        for i in range(1):
            img_file_path = os.path.join(img_dir, str(i))
            img_list = os.listdir(img_file_path)
            # print(img_list)
            for img_name in img_list:
                img_path = os.path.join(img_file_path, img_name)
                img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
                # image_data = [i]
                image_data = []
                image_data.extend(img.flatten())
                print(image_data)
                writer.writerow(image_data)

if __name__ == "__main__":
    convert_img_to_csv(IMG_DIR)


#----------------------------------
# import os
# import csv
#
# def create_csv(dirname):
#     path = './dataset/0/'  #修改为你自己的图片路径即可
#     name = os.listdir(path)
#     with open (dirname+'.csv','w') as csvfile:
#         writer = csv.writer(csvfile)
#         writer.writerow(['data', 'label'])
#         for n in name:
#             if n[-4:] == '.jpg':
#                 print(n)
#                 writer.writerow(['./dataset/'+str(dirname) +'/'+ str(n),'./dataset/' + str(dirname) + 'label/' + str(n[:-4] + '.png')])
#             else:
#                 pass
# #
# if __name__ == "__main__":
#     create_csv('1')



# import csv, os, cv2
# def convert_img_to_csv(img_dir):
#     #设置需要保存的csv路径
#     with open(r"D:\MyCode\Pythonscripts\test\dataset\scan_example.csv","w",newline="") as f:
#         #设置csv文件的列名
#         column_name = ["label"]
#
#
#         column_name.extend(["pixel%d"%i for i in range(432*432)])
#         #将列名写入到csv文件中
#         writer = csv.writer(f)
#         writer.writerow(column_name)
#         #该目录下有9个目录,目录名从0-9
#         for i in range(0):
#             #获取目录的路径
#             img_temp_dir = os.path.join(img_dir,str(i))
#             #获取该目录下所有的文件
#             img_list = os.listdir(img_temp_dir)
#             #遍历所有的文件名称
#             for img_name in img_list:
#                 #判断文件是否为目录,如果为目录则不处理
#                 if not os.path.isdir(img_name):
#                     #获取图片的路径
#                     img_path = os.path.join(img_temp_dir,img_name)
#                     #因为图片是黑白的，所以以灰色读取图片
#                     img = cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
#                     #图片标签
#                     row_data = [i]
#                     #获取图片的像素
#                     row_data.extend(img.flatten())
#                     #将图片数据写入到csv文件中
#                     writer.writerow(row_data)
#
#
# if __name__ == "__main__":
#     #将该目录下的图片保存为csv文件
#     convert_img_to_csv(r"D:\MyCode\Pythonscripts\test\dataset")
