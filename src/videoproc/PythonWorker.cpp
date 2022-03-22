#include <spdlog/spdlog.h>

#include "PythonWorker.h"

void PythonWorker::Run() {
    mWorkerThread = std::jthread(&PythonWorker::Main, this);
}

[[noreturn]] void PythonWorker::Main() {
    py::scoped_interpreter mGuard;
    spdlog::debug("Started python worker main func");

    auto visitor = [&](auto &&msg) { HandleMessage(std::forward<decltype(msg)>(msg)); };
    mRunning = true;

    while (mRunning) {
        std::visit(visitor, mQueue.WaitForMessage());
    }
}

void PythonWorker::HandleMessage(PythonWorkerEnvInit &&envInit) {
    spdlog::debug("Env func");
    py::exec(R"(
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

class Test:
    def __init__(self, name):
        self.name = name

    def get_name(self):
        return self.name

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

    def locate_1_frame(self, num, ecc, mass,size, diam = 15):
        f = tp.locate(self.frames_rescale[num], diam, minmass = self.minmass)
        self.f = f[(f['ecc'] < ecc)&(f['mass'] > mass)&(f['size'] < size)]
        ax = plt.axes([0, 0, 4, 4])
        tp.annotate(self.f, self.frames_rescale[num],ax = ax, imshow_style={'cmap':'viridis'})
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
            )");

    spdlog::info("Did you know that the sqrt(2) = {}?", py::module::import("math").attr("sqrt")(2).cast<double>());
    spdlog::debug("videoproc init successful");
}

void PythonWorker::HandleMessage(PythonWorkerVideoInit &&videoInit) {
    spdlog::debug("Video init func");
    auto locals = py::dict("name"_a = "Jeff", "path"_a = videoInit.path, "file"_a = videoInit.file);
    py::exec(R"(
        test = Test(name)
        new_name = test.get_name()

        vid = Video(path, file)
    )", py::globals(), locals);

    auto message = locals["new_name"].cast<std::string>();
    spdlog::info(message);
}

void PythonWorker::HandleMessage(PythonWorkerQuit &&videoInit) {
    spdlog::debug("Quit");
    mRunning = false;
}