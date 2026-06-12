#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

// WHY
static const std::vector<int> s_hideObjectIDs = {1755, 1813, 1829, 1859, 2866};

static std::unordered_set<GameObject*> s_hiddenObjects;

class $modify(GameObject) { // :heart:
    void setVisible(bool visible) {
        if (s_hiddenObjects.contains(this)) {GameObject::setVisible(false); return;}
        GameObject::setVisible(visible);
    }
    void setOpacity(unsigned char opacity) {
        if (s_hiddenObjects.contains(this)) {GameObject::setOpacity(0); return;}
        GameObject::setOpacity(opacity);
    }
};

class $modify(LevelEditorLayer) {
    struct Fields {
        std::unordered_map<GameObject*, bool> prevInvisible;
        std::unordered_map<GameObject*, bool> prevVisible;
    };

    void hideMatchingObjects() {
        auto objs = this->getAllObjects();
        if (!objs) return;

        for (auto obj : objs->asExt<GameObject*>()) {
            int oid = obj->m_objectID;
            if (std::find(s_hideObjectIDs.begin(), s_hideObjectIDs.end(), oid) != s_hideObjectIDs.end()) {
                if (m_fields->prevInvisible.find(obj) == m_fields->prevInvisible.end()) {
                    m_fields->prevInvisible[obj] = obj->m_isInvisible;
                    m_fields->prevVisible[obj] = obj->isVisible();
                }
                // mark invisible for editor/playtest handling
                obj->m_isInvisible = true;
                obj->setVisible(false);
                obj->setOpacity(0);
                s_hiddenObjects.insert(obj);
            }
        }
    }

    void restoreSavedObjects() {
        for (auto& [obj, invisible] : m_fields->prevInvisible) {
            obj->m_isInvisible = invisible;
            s_hiddenObjects.erase(obj);
            if (m_fields->prevVisible.contains(obj)) {
                obj->setVisible(m_fields->prevVisible[obj]);
            }
        }

        m_fields->prevInvisible.clear();
        m_fields->prevVisible.clear();
    }

    void onPlaytest() {
        LevelEditorLayer::onPlaytest();
        hideMatchingObjects();
    }

#ifndef GEODE_IS_WINDOWS
    void onPausePlaytest() {
        restoreSavedObjects();
        LevelEditorLayer::onPausePlaytest();
    }
#endif

#ifndef GEODE_IS_WINDOWS
    void onResumePlaytest() {
        LevelEditorLayer::onResumePlaytest();
        hideMatchingObjects();
    }
#endif

    void onStopPlaytest() {
        restoreSavedObjects();
        LevelEditorLayer::onStopPlaytest();
    }
};
