//
// Created by bp1 on 13/11/2022.
//

#include "attysjava2cpp.h"
#include "util.h"
#include "ecg_rr_det.h"
#include "Iir.h"
#include "hr_sham.h"

constexpr long MAX_HR_FILESIZE = 100000000; // 100MB

////////////////////////////////
// Heartrate callback from java
std::vector<std::function<void(float)>> attysHRCallbacks;

void registerAttysHRCallback(const std::function<void(float)> &f) {
    attysHRCallbacks.emplace_back(f);
}

void doAllHRCallbacks(float bpm) {
    for (auto &cb: attysHRCallbacks) {
        cb(bpm);
    }
}





void writeHR2file(float hr) {
    const std::string path = getAttysHRfilepath();
    if (path.empty()) {
        ALOGE("HR file path not set");
        return;
    }
    FILE* hrFile = fopen(path.c_str(), "at");
    if (nullptr == hrFile) {
        ALOGE("Cannot write to HR file: %s", path.c_str());
        return;
    }
    fseek(hrFile, 0L, SEEK_END);
    long sz = ftell(hrFile);
    ALOGV("Writing to HR file: %s, size = %ld", path.c_str(), sz);
    if (sz > MAX_HR_FILESIZE) {
        fclose(hrFile);
        ALOGV("HR file %s too large: size = %ld", path.c_str(), sz);
        return;
    }
    struct timeval tv = {};
    gettimeofday(&tv, nullptr);
    const long epo = (long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
    ALOGV("Writing to HR file: %ld, %.1f", epo, hr);
    const int r = fprintf(hrFile, "%ld\t%.1f\n", epo, hr);
    if (r < 0) {
        ALOGE("Could not write to heartrate-file!");
    }
    fclose(hrFile);
}


// fakeHR with 250Hz sampling rate
FakeHR fakeHR;

class MyHRCallBack: public ECG_rr_det::RRlistener {
public:
    void hasRpeak(long,
                  float bpm,
                  double,
                  double) override {
        writeHR2file(bpm);
        if (!isSham()) {
            ALOGV("HR = %f",bpm);
            doAllHRCallbacks(bpm);
        } else {
            ALOGV("HR = fake",bpm);
            fakeHR.setEnabled();
        }
    }
};

MyHRCallBack hrCallBack;
ECG_rr_det rrDet(&hrCallBack);

/////////////////////////////////
// Raw data callback from JAVA
std::vector<std::function<void(float)>> attysDataCallbacks;

void registerAttysDataCallback(const std::function<void(float)> &f) {
    ALOGV("Registered callback # %lu for Attys data.", (long) attysDataCallbacks.size());
    attysDataCallbacks.emplace_back(f);
}

Iir::Butterworth::BandStop<2> iirnotch;

extern "C"
JNIEXPORT void JNICALL
Java_tech_glasgowneuro_attyshrv_ANativeActivity_dataUpdate(JNIEnv *, jclass, jlong instance, jfloat data) {
    data = iirnotch.filter(data);
    rrDet.detect(data);
    if (isSham()) {
        const float bpm = fakeHR.getFakeHR();
        if (bpm > 0) {
            ALOGV("fake HR = %f",bpm);
            doAllHRCallbacks(bpm);
        }
    }
    for (auto &v : attysDataCallbacks) {
        v(data);
    }
}

////////////////////////////////////////////////
// Init callback that the Attys has been started
std::vector<std::function<void(float)>> attysInitCallbacks;

void registerAttysInitCallback(const std::function<void(float)> &f) {
    attysInitCallbacks.emplace_back(f);
}

extern "C"
JNIEXPORT void JNICALL
Java_tech_glasgowneuro_attyshrv_ANativeActivity_initJava2CPP(JNIEnv *env,
                                                              jclass clazz,
                                                              jfloat fs) {
    ALOGV("Settting up the notch filter and HR detector: fs = %f", fs);
    for(auto &v : attysInitCallbacks) {
        v(fs);
    }
    if (fs < 125) return;
    iirnotch.setup(fs, 50, 2.5);
    rrDet.init(fs);
    fakeHR.init(fs);
    int nOn = 0;
    int nControl = 0;
    for(int i = 0; i < 50; i++) {
        bool b = calcRandom(i);
        if (b) {
            nOn++;
        } else {
            nControl++;
        }
    }
    ALOGV("Trial: on = %d, off = %d",nOn,nControl);
}

//////////////////////////////////
// filename

std::string attysHRfilepath;

std::string getAttysHRfilepath() {
    return  attysHRfilepath;
}

extern "C"
JNIEXPORT void JNICALL
Java_tech_glasgowneuro_attyshrv_ANativeActivity_setHRfilePath(JNIEnv *env, jclass clazz,
                                                              jstring path) {
    const char *fnUTF = env->GetStringUTFChars(path, NULL);
    ALOGV("Callback from onCreate for HR with path: %s",fnUTF);
    attysHRfilepath = std::string(fnUTF);
    env->ReleaseStringUTFChars(path, fnUTF);
}


void unregisterAllAttysCallbacks() {
    ALOGV("Unregistering all Attys callbacks");
    attysHRCallbacks.clear();
    attysDataCallbacks.clear();
    attysInitCallbacks.clear();
}
