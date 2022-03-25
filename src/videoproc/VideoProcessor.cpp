#include "VideoProcessor.h"
#include "messages/messages.h"

void VideoProcessor::Init() {
    spdlog::info("Initializing video processor module");
    mMessageQueue.Send(PythonWorkerRunString{
            .string = R"(
import pims
import numpy as np
import trackpy as tp
import pandas as pd
import matplotlib.pyplot as plt
from skimage import data, img_as_float
from skimage import exposure
from PIL import Image

from tifffile import imsave
import os

class Video:
    def __init__(self, path, file):
        self.file = file
        self.path = path
        self.frames = np.array(pims.as_grey(pims.TiffStack(self.path + self.file)))

        self.frames_rescale = self.frames
        self.minmass = 1e5
        self.f = pd.DataFrame()
        self.t = pd.DataFrame()
        self.d = pd.DataFrame()
        self.raw_t = pd.DataFrame()
        self.up_perc = 99.99

    def __str__(self):
        return "Video"

    def locate_1_frame(self, num, ecc, mass,size, diam = 15):
        f = tp.locate(self.frames_rescale[num], diam, minmass = self.minmass)
        self.f = f[(f['ecc'] < ecc)&(f['mass'] > mass)&(f['size'] < size)]

        fig, ax = plt.subplots()
        tp.annotate(self.f, self.frames_rescale[num], ax = ax, imshow_style = {'cmap':'viridis'})
        plt.show()
        plt.savefig('figure.png')

    def locate_all(self, ecc, mass, size, diam = 15):
        f = tp.batch(self.frames_rescale, diam, minmass=self.minmass)
        self.f = f[(f['ecc'] < ecc)&(f['mass'] > mass)&(f['size'] < size)]
    def link(self,  ecc, mass, size,search_range = 10, memory = 3):
        self.raw_t = tp.link(self.f, search_range, memory=memory)
        self.t = self.raw_t[(self.raw_t['ecc'] < ecc)&(self.raw_t['mass'] > mass)&(self.raw_t['size'] < size)]
    def get_drift(self):
        self.d = tp.compute_drift(self.t)
        self.d.plot()
        plt.show()
    def filter_traj(self, min_len = 20):
        self.t = tp.filter_stubs(self.raw_t, min_len)
        print('Before:', self.raw_t['particle'].nunique())
        print('After:', self.t['particle'].nunique())
    def vel_profile(self, ax):

        N = len(self.t['particle'].unique())
        y_array = np.zeros(N)
        vel_array = np.zeros(N)

        for i in range(N):
            length = len(self.t[self.t['particle'] == i])
            if length != 0:
                y_array[i] = np.mean(self.t[self.t['particle'] == i]['y'])
                xmax = max(self.t[self.t['particle'] == i]['x'])

                xmin = min(self.t[self.t['particle'] == i]['x'])
                vel_array[i] = (xmax - xmin)/length
        y_array = y_array[y_array != 0]
        vel_array = vel_array[vel_array != 0]
        #plt.hist(vel_array, bins = 12)
        ax.scatter(y_array, vel_array)
        ax.set_xlabel('Положение на оси y')
        ax.set_ylabel('Скорость')
        pass
        return y_array, vel_array

    def get_size(self,fps, scale, cutoff = 0):
        x10_scale = 1400/621
        x20_scale = 600/594
        #fps = 6.66
        em = tp.emsd(self.t, scale, fps) #Микрон на пиксель

        #Для 10x - 1400 мкм на 621 пиксель
        #Для 20x - 600 мкм на 594 пиксель
        plt.figure()
        plt.ylabel(r'$\langle \Delta r^2 \rangle$ [$\mu$m$^2$]')
        plt.xlabel('lag time $t$');
        res = tp.utils.fit_powerlaw(em[cutoff:])  # performs linear best fit in log space, plots]

        A = float(res['A'])
        print(res)

        eta = 0.89e-3
        k = 1.380649e-23
        T = 273 + 25
        D = 1e-12*A/4 # -12 степень - потому что мкм**2
        R = k*T/(6*np.pi*D*eta)
        diam = 2*R*1e9
        print('diameter = ', diam, ' nm')
)"});
}

void VideoProcessor::Test(const std::string &path, const std::string &file) {
    spdlog::info("Instantiating Video and testing one frame");
    mMessageQueue.Send(PythonWorkerRunString{
            .string = R"(
vid = Video(path, file)
)",
            .strVariables{
                    {"path", path},
                    {"file", file}
            }
    });

    mMessageQueue.Send(PythonWorkerRunString{
            .string = R"(
minm = 1e3 #1e2 = 100
ecc = 0.5
vid.minmass = minm
mass = minm
size = 5
diametr = 19

#Эта функция для подбора параметров на одном кадре. Изменяем параметры (в основном, mass),
#пока картинка не станет хорошей, и только тогда запускаем locate_all
vid.locate_1_frame(0, ecc, mass, size, diametr)
)"
    });
}