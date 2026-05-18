#pragma once

#include <string>
#include <unordered_map>

class GRAMS_TOF_Config {

public:
    static GRAMS_TOF_Config& instance();

    static bool loadDefaultConfig();
    void setConfigFile(const std::string& filename);
    void setVaultPath(const std::string& path) { vaultPath_ = path; }
    void clearVaultPath() { vaultPath_ = ""; }

    std::string getString(const std::string& section, const std::string& key) const;
    std::string getAbsolutePath(const std::string& section, const std::string& key) const;
    std::string getFileStem(const std::string& section, const std::string& key) const;
    std::string getFileStemWithDir(const std::string& section, const std::string& key) const;
    std::string getSubDir(const std::string& subDirName) const;
    std::string getFileByTimestamp(const std::string& absDir, const std::string& prefix, const std::string& timestamp, const std::string& ext="", bool strict=false) const; 
    std::string makeFilePathWithTimestamp(const std::string& absDir, const std::string& prefix, const std::string& timestamp, const std::string& ext="") const;

    std::string getLatestTimestamp(const std::string& absDir, const std::string& prefix, const std::string& suffix="", bool strict=false) const;
    std::string getCurrentTimestamp() const;
    void copyOrLink(const std::string& srcPath, const std::string& dstPath, bool symlink=true) const;

    int getInt(const std::string& section, const std::string& key) const;
    double getDouble(const std::string& section, const std::string& key) const;

    const std::string& getConfigFilePath() const;

    // TOFDATA directories
    std::string getConfigDir() const;        // returns %CDIR
    std::string getTOFDataDir() const;       // returns $TOFDATA
    std::string getSTG0Dir() const;          // $TOFDATA/stg0
    std::string getSTG1Dir() const;          // $TOFDATA/stg1
    std::string getSTG2Dir() const;          // $TOFDATA/stg2
    std::string getLogDir() const;           // $TOFDATA/log
    std::string getHistDir() const;          // $TOFDATA/hist
    std::string getDiscDir() const;          // $TOFDATA/disc
    std::string getQDCDir() const;           // $TOFDATA/qdc
    std::string getTDCDir() const;           // $TOFDATA/tdc
    std::string getCalibrationDir() const;   // $TOFDATA/calibration

private:
    GRAMS_TOF_Config() = default;
    GRAMS_TOF_Config(const GRAMS_TOF_Config&) = delete;
    GRAMS_TOF_Config& operator=(const GRAMS_TOF_Config&) = delete;

    bool load(const std::string& filename);

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_;
    std::string configDir_;
    std::string configFilePath_;
    std::string tofdataDir_;
    std::string vaultPath_;
    bool loaded_ = false;

    std::string substituteVariables(const std::string& value) const;
};

inline std::string GRAMS_TOF_Config::getConfigDir()  const { return configDir_; }
inline std::string GRAMS_TOF_Config::getTOFDataDir() const { return tofdataDir_; }
inline std::string GRAMS_TOF_Config::getSTG0Dir()    const { return vaultPath_.empty() ? tofdataDir_ + "/stg0" : vaultPath_; }
inline std::string GRAMS_TOF_Config::getSTG1Dir()    const { return vaultPath_.empty() ? tofdataDir_ + "/stg1" : vaultPath_; }
inline std::string GRAMS_TOF_Config::getSTG2Dir()    const { return vaultPath_.empty() ? tofdataDir_ + "/stg2" : vaultPath_; }
inline std::string GRAMS_TOF_Config::getLogDir()     const { return vaultPath_.empty() ? tofdataDir_ + "/log"  : vaultPath_; }
inline std::string GRAMS_TOF_Config::getHistDir()    const { return vaultPath_.empty() ? tofdataDir_ + "/hist" : vaultPath_; }
inline std::string GRAMS_TOF_Config::getDiscDir()    const { return vaultPath_.empty() ? tofdataDir_ + "/disc" : vaultPath_; }
inline std::string GRAMS_TOF_Config::getQDCDir()     const { return vaultPath_.empty() ? tofdataDir_ + "/qdc"  : vaultPath_; }
inline std::string GRAMS_TOF_Config::getTDCDir()     const { return vaultPath_.empty() ? tofdataDir_ + "/tdc"  : vaultPath_; }
inline std::string GRAMS_TOF_Config::getCalibrationDir() const { return vaultPath_.empty() ? tofdataDir_ + "/calibration" : vaultPath_; }

