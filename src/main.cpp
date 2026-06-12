#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

// WHY
static const std::vector<int> s_hideObjectIDs = {1755, 1813, 1829, 1859, 2866};

class $modify(LevelEditorLayer) {
    struct Fields {
        gd::unordered_map<int, unsigned char> prevOpacity;
        gd::unordered_map<int, bool> prevVisible;
    };

    void hideMatchingObjects() {
        auto objs = this->getAllObjects();
        if (!objs) return;

        for (size_t i = 0; i < objs->count(); ++i) {
            auto obj = static_cast<GameObject*>(objs->objectAtIndex(i));
            if (!obj) continue;

            // compare object type id
            int oid = obj->m_objectID;
            if (std::find(s_hideObjectIDs.begin(), s_hideObjectIDs.end(), oid) != s_hideObjectIDs.end()) {
                int uid = obj->m_uniqueID;
                if (m_fields->prevOpacity.find(uid) == m_fields->prevOpacity.end()) {
                    unsigned char cur = 255;
                    // bad code my beloved
                    cur = obj->getOpacity();
                    bool vis = obj->isVisible();
                    m_fields->prevOpacity[uid] = cur;
                    m_fields->prevVisible[uid] = vis;
                }
                // editor logic still works THATS why opacity'ing to 0
                obj->setOpacity(0);
            }
        }
    }

    void restoreSavedObjects() {
        // restore objs by their id
        for (auto it = m_fields->prevOpacity.begin(); it != m_fields->prevOpacity.end(); ++it) {
            int uid = it->first;
            unsigned char prev = it->second;
            bool prevVis = false;
            auto visIt = m_fields->prevVisible.find(uid);
            if (visIt != m_fields->prevVisible.end()) prevVis = visIt->second;
            auto obj = this->findGameObject(uid);
            if (!obj) continue;
            obj->setOpacity(prev);
            obj->setVisible(prevVis);
        }

        m_fields->prevOpacity.clear();
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
