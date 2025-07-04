#include "../header/Simulator.h"

void Simulator::loadSharedObjectsFromFolder(const std::string& folderPath) {
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".so") {
            std::string fullPath = folderPath + "/" + filename;
            std::cout << "Loading: " << fullPath << std::endl;

            void* handle = dlopen(fullPath.c_str(), RTLD_NOW);
            if (!handle) {
                std::cerr << "Failed to load " << filename << ": " << dlerror() << std::endl;
                continue;
            }

            // Example: Try to get a known symbol (like `init_plugin`)
            plugin_init_func init = (plugin_init_func)dlsym(handle, "init_plugin");
            const char* dlsym_error = dlerror();
            if (dlsym_error) {
                std::cerr << "Cannot load symbol 'init_plugin': " << dlsym_error << std::endl;
            } else {
                std::cout << "Calling init_plugin from " << filename << std::endl;
                init();
            }

            // Optional: keep the handle if you need to use it later
            // Otherwise, unload now
             dlclose(handle);
        }
    }

    closedir(dir);
}