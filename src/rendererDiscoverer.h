#ifndef RENDERER_DISCOVERER_H
#define RENDERER_DISCOVERER_H

#include <map>
#include <string>
#include <mutex>
#include <stdexcept>
#include <vlc/vlc.h>
#include "audiowrapper.h"

class RendererDiscoverer {
public:
    RendererDiscoverer(libvlc_instance_t* instance, AudioWrapper* audioWrapper);
    ~RendererDiscoverer();

    void start();
    std::map<std::pair<std::string, std::string>, libvlc_renderer_item_t*> getRenderers();

private:
    AudioWrapper* parent;
    std::list<libvlc_renderer_discoverer_t*> discoverers;
    std::list<libvlc_event_manager_t *> eventManagers;
    static std::map<std::pair<std::string, std::string>, libvlc_renderer_item_t*> renderers;
    static std::mutex renderersMutex;  

    static void onItemAdded(const libvlc_event_t* event, void* userData);
    static void onItemDeleted(const libvlc_event_t* event, void* userData);
};

#endif // RENDERER_DISCOVERER_H