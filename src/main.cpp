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

static std::unordered_set<int> s_hiddenUniqueIDs;

class $modify(GameObject) { // :heart:
    void setVisible(bool visible) {
        if (s_hiddenUniqueIDs.contains(this->m_uniqueID)) {GameObject::setVisible(false); return;}
        GameObject::setVisible(visible);
    }
    void setOpacity(unsigned char opacity) {
        if (s_hiddenUniqueIDs.contains(this->m_uniqueID)) {GameObject::setOpacity(0); return;}
        GameObject::setOpacity(opacity);
    }
};

class $modify(LevelEditorLayer) {
    struct Fields {
        std::unordered_map<int, bool> prevInvisible;
        std::unordered_map<int, bool> prevVisible;
    };

    void hideMatchingObjects() {
        auto objs = this->getAllObjects();
        if (!objs) return;

        for (auto obj : objs->asExt<GameObject*>()) {
            int oid = obj->m_objectID;
            if (std::find(s_hideObjectIDs.begin(), s_hideObjectIDs.end(), oid) != s_hideObjectIDs.end()) {
                int uid = obj->m_uniqueID;
                if (m_fields->prevInvisible.find(uid) == m_fields->prevInvisible.end()) {
                    // save previous invisible state
                    m_fields->prevInvisible[uid] = obj->m_isInvisible;
                    m_fields->prevVisible[uid] = obj->isVisible();
                }
                // mark invisible for editor/playtest handling
                obj->m_isInvisible = true;
                obj->setVisible(false);
                obj->setOpacity(0);
                s_hiddenUniqueIDs.insert(uid);
            }
        }
    }

    void restoreSavedObjects() {
        // restore objs by their id
        for (auto it = m_fields->prevInvisible.begin(); it != m_fields->prevInvisible.end(); ++it) {
            int uid = it->first;
            auto obj = this->findGameObject(uid);
            if (!obj) continue;
            obj->m_isInvisible = it->second;
            s_hiddenUniqueIDs.erase(uid);
            if (m_fields->prevVisible.contains(uid)) {
                obj->setVisible(m_fields->prevVisible[uid]);
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
