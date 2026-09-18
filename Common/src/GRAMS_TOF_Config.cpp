#include "GRAMS_TOF_Config.h"
#include "GRAMS_TOF_Logger.h"
#include "GRAMS_TOF_RuntimeError.h"
#include "INIReader.h"

#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>
#include <algorithm>

namespace {
    // Define the relative path to the config file from $GLIB
    const std::filesystem::path RELATIVE_CONFIG_PATH = std::filesystem::path("config") / "config.ini";
}

GRAMS_TOF_Config& GRAMS_TOF_Config::instance() {
    static GRAMS_TOF_Config instance;
    return instance;
}

bool GRAMS_TOF_Config::loadDefaultConfig() {
    GRAMS_TOF_Config& config = instance();
    if (config.loaded_) return true; // Already loaded, skip.

    const char* glibPath = std::getenv("GLIB");
    if (!glibPath) {
        Logger::instance().error("[Config] GLIB environment variable not set. Default config not loaded.");
        return false;
    }

    const char* tofdata = std::getenv("TOFDATA");
    if (!tofdata) {
        Logger::instance().error("[Config] TOFDATA environment variable is not set. Cannot continue.");
        return false;
    } else {
        config.tofdataDir_ = tofdata;
    }

    std::filesystem::path defaultPath = std::filesystem::path(glibPath) / RELATIVE_CONFIG_PATH;
    std::string defaultPathStr = defaultPath.string();

    if (config.load(defaultPathStr)) {
        config.configFilePath_ = defaultPathStr;
        config.loaded_ = true;
        Logger::instance().info("[Config] Default config loaded from GLIB: {}", defaultPathStr);
        return true;
    } else {
        Logger::instance().warn("[Config] Failed to load default config from {}", defaultPathStr);
        return false;
    }
}

void GRAMS_TOF_Config::setConfigFile(const std::string& filename) {
    if (loaded_) {
        data_.clear();
    }
    if (!load(filename)) {
        // The empty string path was failing here!
        throw GRAMS_TOF_RuntimeError("[Config] Failed to load config file: " + filename);
    }
    configFilePath_ = filename;
    loaded_ = true;
}

bool GRAMS_TOF_Config::load(const std::string& filename) {
    configDir_ = std::filesystem::path(filename).parent_path().string();

    INIReader reader(filename);
    if (reader.ParseError() < 0) {
        return false;
    }

    data_.clear();
    for (const auto& section : reader.Sections()) {
        for (const auto& key : reader.GetKeys(section)) {
            std::string rawValue = reader.Get(section, key, "");
            data_[section][key] = substituteVariables(rawValue);
        }
    }

    return true;
}

std::string GRAMS_TOF_Config::substituteVariables(const std::string& value) const {
    std::string result = value;
    size_t pos;

    if ((pos = result.find("%CDIR%")) != std::string::npos) result.replace(pos, 6, configDir_);
    if ((pos = result.find("%PWD%")) != std::string::npos) result.replace(pos, 5, ".");
    if ((pos = result.find("%HOME%")) != std::string::npos) {
        const char* home = std::getenv("HOME");
        if (home) result.replace(pos, 6, home);
    }
    if ((pos = result.find("$TOFDATA")) != std::string::npos) {
        const char* tofdata = std::getenv("TOFDATA");
        if (tofdata) result.replace(pos, 8, tofdata);
        else throw GRAMS_TOF_RuntimeError("[Config] TOFDATA environment variable not set.");
    }

    return result;
}

std::string GRAMS_TOF_Config::getString(const std::string& section, const std::string& key) const {
    auto secIt = data_.find(section);
    if (secIt == data_.end()) throw GRAMS_TOF_RuntimeError("[Config] Missing section: " + section);
    auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) throw GRAMS_TOF_RuntimeError("[Config] Missing key: " + key);
    return keyIt->second;
}

std::string GRAMS_TOF_Config::getAbsolutePath(const std::string& section, const std::string& key) const {
    std::string pathStr = getString(section, key);
    std::filesystem::path p(pathStr);
    return std::filesystem::absolute(p).string();
}

std::string GRAMS_TOF_Config::getFileStem(const std::string& section, const std::string& key) const {
    std::string pathStr = getString(section, key); 
    return std::filesystem::path(pathStr).stem().string();
}

std::string GRAMS_TOF_Config::getFileStemWithDir(const std::string& section, const std::string& key) const {
    std::string pathStr = getString(section, key);
    std::filesystem::path p(pathStr);
    return (p.parent_path() / p.stem()).string(); 
}

std::string GRAMS_TOF_Config::getFileByTimestamp(
    const std::string& absDir,
    const std::string& prefix,
    const std::string& timestamp,
    const std::string& ext,
    bool strict) const
{
    namespace fs = std::filesystem;

    fs::path dir(absDir);
    if (!fs::exists(dir) || !fs::is_directory(dir))
        throw GRAMS_TOF_RuntimeError("[Config] Directory does not exist: " + dir.string());

    std::string pattern;
    if (strict) pattern = prefix + "_" + timestamp + ext + "$"; 
    else        pattern = prefix + "_" + timestamp + R"(.*)" + ext;
    std::regex re(pattern);

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        const std::string filename = entry.path().filename().string();
        if (std::regex_match(filename, re))
            return entry.path().string();
    }

    throw GRAMS_TOF_RuntimeError(
        "[Config] No file found for timestamp " + timestamp + 
        " with ext'" + ext + "' in " + dir.string()
    );
}


std::string GRAMS_TOF_Config::makeFilePathWithTimestamp(
    const std::string& absDir,
    const std::string& prefix,
    const std::string& timestamp,
    const std::string& ext ) const
{
    namespace fs = std::filesystem;
    fs::path dir(absDir);
    if (!fs::exists(dir)) fs::create_directories(dir);

    std::string dotExt = ext.empty() ? "" : (ext.front() == '.' ? ext : "." + ext);

    return (dir / (prefix + "_" + timestamp + dotExt)).string();
}


std::string GRAMS_TOF_Config::getLatestTimestamp(
    const std::string& absDir,
    const std::string& prefix,
    const std::string& suffix,
    bool strict) const 
{
    namespace fs = std::filesystem;

    fs::path dir(absDir);
    if (!fs::exists(dir) || !fs::is_directory(dir))
        throw GRAMS_TOF_RuntimeError("[Config] Directory does not exist: " + dir.string());

    std::string pattern;
    if (strict) pattern = prefix + R"(_(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.\d+Z))" + suffix + "$";
    else        pattern = prefix + R"(_(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.\d+Z))" + suffix + R"(.*)";
    std::regex re(pattern);

    std::string latestTs;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        std::smatch match;
        std::string filename = entry.path().filename().string();
        
        if (std::regex_match(filename, match, re)) {
            const std::string& ts = match[1].str();
            if (latestTs.empty() || ts > latestTs) {
                latestTs = ts;
            }
        }
    }

    if (latestTs.empty()) {
        throw GRAMS_TOF_RuntimeError("[Config] No matching file found with suffix '" + suffix + "' in " + dir.string());
    }

    return latestTs;
}


std::string GRAMS_TOF_Config::getCurrentTimestamp() const
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto secs = time_point_cast<std::chrono::seconds>(now);
    auto frac = duration_cast<milliseconds>(now - secs).count();

    std::time_t t = system_clock::to_time_t(secs);
    std::tm tm = *std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
        << "." << frac << "Z";
    return oss.str();
}

/*
void GRAMS_TOF_Config::copyOrLink(const std::string& srcPath, const std::string& dstPath, bool symlink) const
{
    namespace fs = std::filesystem;
    fs::path src(srcPath);
    fs::path dst(dstPath);

    Logger::instance().info(
        "[Config] copyOrLink: src='{}', dst='{}', mode={}",
        src.string(),
        dst.string(),
        symlink ? "symlink" : "copy"
    );

    std::error_code ec;
    if (fs::equivalent(src, dst, ec)) {
        Logger::instance().debug("[Config] Source and destination are the same path. Skipping copyOrLink: {}", src.string());
        return;
    }

    if (!fs::exists(src)) throw GRAMS_TOF_RuntimeError("[Config] Source file does not exist: " + src.string());

    if (fs::exists(dst) || fs::is_symlink(dst)) {
        fs::remove_all(dst);
    }

    if (symlink) {
        fs::create_symlink(src, dst);
    } else {
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
    }
}
*/

void GRAMS_TOF_Config::copyOrLink(const std::string& srcPathStr, const std::string& dstPathStr, bool symlink) const
{
    namespace fs = std::filesystem;
    fs::path src(srcPathStr);
    fs::path dst(dstPathStr);

    Logger::instance().info(
        "[Config] copyOrLink: src='{}', dst='{}', mode={}",
        src.string(),
        dst.string(),
        symlink ? "symlink" : "copy"
    );

    std::error_code ec;

    // Do nothing if the source does not exist
    if (!fs::exists(src) && !fs::is_symlink(src)) {
        Logger::instance().warn("[Config] copyOrLink: Source does not exist: {}", src.string());
        return;
    }

    // (3) Skip if the source and destination point to the exact same file/directory
    if (fs::exists(dst) && fs::equivalent(src, dst, ec)) {
        Logger::instance().debug("[Config] copyOrLink: Source and destination are equivalent. Skipping: {}", src.string());
        return;
    }

    // (1) Handle single file input
    if (!fs::is_directory(src)) {
        fs::path targetFilePath;

        if (dst.has_extension()) {
            // Check if any component in the parent path is blocked by a non-directory file
            if (fs::exists(dst.parent_path()) && !fs::is_directory(dst.parent_path())) {
                Logger::instance().error("[Config] copyOrLink: Parent path exists but is not a directory: {}", dst.parent_path().string());
                return;
            }
            targetFilePath = dst;

        } else {
            // Safety guard: Ensure 'dst' is not accidentally a regular file
            if (fs::exists(dst) && !fs::is_directory(dst)) {
                Logger::instance().error("[Config] copyOrLink: Expected directory at dst, but found a regular file: {}", dst.string());
                return;
            }
            targetFilePath = dst / src.filename();
        }

        // Safely create parent directories if they do not exist
        if (!fs::exists(targetFilePath.parent_path())) {
            fs::create_directories(targetFilePath.parent_path());
        }

        // Re-check equivalence after resolving the final target file path
        if (fs::exists(targetFilePath) && fs::equivalent(src, targetFilePath, ec)) {
            return;
        }

        // (4) Remove existing file or symlink at target location for overwriting
        if (fs::exists(targetFilePath) || fs::is_symlink(targetFilePath)) {
            fs::remove(targetFilePath);
        }

        if (symlink) {
            fs::create_symlink(src, targetFilePath);
        } else {
            fs::copy_file(src, targetFilePath, fs::copy_options::overwrite_existing);
        }
        return;
    }

    // (2) Handle directory input
    // Safety guard: Ensure 'dst' is not a regular file when 'src' is a directory
    if (fs::exists(dst) && !fs::is_directory(dst)) {
        Logger::instance().error("[Config] copyOrLink: Cannot copy/link directory src to a file dst: {}", dst.string());
        return;
    }

    fs::path targetDir = dst;
    if (!fs::exists(targetDir)) {
        fs::create_directories(targetDir);
    }

    for (const auto& entry : fs::directory_iterator(src)) {
        // Skip subdirectories since recursive directory copying/linking is not required
        if (entry.is_directory()) continue;

        fs::path fileSrc = entry.path();
        fs::path fileDst = targetDir / fileSrc.filename();

        // (3) Check equivalence for each file
        if (fs::exists(fileDst) && fs::equivalent(fileSrc, fileDst, ec)) {
            continue;
        }

        // (4) Remove existing file or symlink before overwriting
        if (fs::exists(fileDst) || fs::is_symlink(fileDst)) {
            fs::remove(fileDst);
        }

        if (symlink) {
            fs::create_symlink(fileSrc, fileDst);
        } else {
            fs::copy_file(fileSrc, fileDst, fs::copy_options::overwrite_existing);
        }
    }
}

void GRAMS_TOF_Config::linkVaultFileToOriginalDir(const std::string& vaultFilePath, const std::string& originalDir) const 
{
    namespace fs = std::filesystem;
    fs::path srcPath(vaultFilePath);
    
    if (!fs::exists(srcPath)) return;

    fs::path targetDir(originalDir);
    if (!fs::exists(targetDir)) {
        fs::create_directories(targetDir);
    }

    fs::path dstPath = targetDir / srcPath.filename();
    
    copyOrLink(srcPath.string(), dstPath.string(), true);
}

int GRAMS_TOF_Config::getInt(const std::string& section, const std::string& key) const {
    return std::stoi(getString(section, key));
}

double GRAMS_TOF_Config::getDouble(const std::string& section, const std::string& key) const {
    return std::stod(getString(section, key));
}

const std::string& GRAMS_TOF_Config::getConfigFilePath() const {
    return configFilePath_;
}

