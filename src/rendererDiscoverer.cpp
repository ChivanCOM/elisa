#include "rendererDiscoverer.h"

std::list<libvlc_renderer_discoverer_t *> discoverers;
std::list<libvlc_event_manager_t *> eventManagers;
std::map<std::pair<std::string, std::string>, libvlc_renderer_item_t *> RendererDiscoverer::renderers;
std::mutex RendererDiscoverer::renderersMutex;
AudioWrapper *parent;

RendererDiscoverer::RendererDiscoverer(libvlc_instance_t *instance, AudioWrapper *audioWrapper)
{
    parent = audioWrapper;
    std::lock_guard<std::mutex> guard(renderersMutex);
    renderers[{"Local", "None"}] = nullptr;
    libvlc_rd_description_t **pp_services = nullptr;

    int num_services = 0;
    num_services = libvlc_renderer_discoverer_list_get(instance, &pp_services);
    if (num_services > 0 && pp_services != nullptr)
    {
        for (int i = 0; i < num_services; ++i)
        {
            libvlc_rd_description_t *service = pp_services[i];
            libvlc_renderer_discoverer_t *discoverer = libvlc_renderer_discoverer_new(instance, service->psz_name);
            if (discoverer)
            {
                libvlc_event_manager_t *eventManager = libvlc_renderer_discoverer_event_manager(discoverer);
                libvlc_event_attach(eventManager, libvlc_RendererDiscovererItemAdded, this->onItemAdded, this);
                libvlc_event_attach(eventManager, libvlc_RendererDiscovererItemDeleted, this->onItemDeleted, this);
                discoverers.push_back(discoverer);
                eventManagers.push_back(eventManager);
            }
        }
    }
    libvlc_renderer_discoverer_list_release(pp_services, num_services);
}

RendererDiscoverer::~RendererDiscoverer()
{
    for (auto discoverer : discoverers)
    {
        libvlc_renderer_discoverer_stop(discoverer);
        libvlc_renderer_discoverer_release(discoverer);
    }
    for (auto renderer : renderers)
    {
        if (renderer.second)
        {
            libvlc_renderer_item_release(renderer.second);
        }
    }
    parent = nullptr;
}

void RendererDiscoverer::start()
{
    for (auto discoverer : discoverers)
    {
        libvlc_renderer_discoverer_start(discoverer);
    }
}

std::map<std::pair<std::string, std::string>, libvlc_renderer_item_t *> RendererDiscoverer::getRenderers()
{
    return renderers;
}

void RendererDiscoverer::onItemAdded(const libvlc_event_t *event, void *userData)
{
    libvlc_renderer_item_t *renderer = libvlc_renderer_item_hold(
        event->u.renderer_discoverer_item_added.item);
    std::string rendererName(libvlc_renderer_item_name(renderer));
    std::string rendererType(libvlc_renderer_item_type(renderer));
    std::lock_guard<std::mutex> guard(renderersMutex);
    renderers[{rendererName, rendererType}] = renderer;
    RendererDiscoverer *self = static_cast<RendererDiscoverer *>(userData);
    self->parent->renderersChanges();
}

void RendererDiscoverer::onItemDeleted(const libvlc_event_t *event, void *userData)
{
    libvlc_renderer_item_t *renderer = event->u.renderer_discoverer_item_deleted.item;
    std::string rendererName(libvlc_renderer_item_name(renderer));
    std::string rendererType(libvlc_renderer_item_type(renderer));
    auto existing = renderers.find({rendererName, rendererType});
    if (existing != renderers.end())
    {
        std::lock_guard<std::mutex> guard(renderersMutex);
        renderers.erase({rendererName, rendererType});
        auto *self = static_cast<RendererDiscoverer *>(userData);
        self->parent->renderersChanges();
    }
}