#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/component.hpp"
#include "primwalk/systems/system.hpp"

// std
#include <memory>
#include <unordered_map>

namespace pw {
  class PW_API SystemManager {
  public:
    // Remove copy/move constructors
    SystemManager(const SystemManager&) = delete;
    SystemManager(SystemManager&&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;
    SystemManager& operator=(SystemManager&&) = delete;

    static SystemManager& Get();

    template <typename T, typename... Args>
    std::shared_ptr<T> registerSystem(Args&&... args) {
      const char* typeName = typeid(T).name();

      assert(m_Systems.find(typeName) == m_Systems.end() && "ERROR: Attempting to register an already registered system!");

      auto system = std::make_shared<T>(std::forward<Args>(args)...);
      m_Systems.insert({ typeName, system.get() });
      return system;
    }

    template <typename T>
    void setSignature(component_signature signature) {
      const char* typeName = typeid(T).name();

      assert(m_Systems.find(typeName) != m_Systems.end() && "ERROR: Attempted use of unregistered system!");

      m_Signatures.insert({ typeName, signature });
    }

    void entityDestroyed(entity_id entity);
    void entitySignatureChanged(entity_id entity, component_signature signature);

  private:
    SystemManager() = default;
    ~SystemManager() = default;

    std::unordered_map<const char*, component_signature> m_Signatures{};
    std::unordered_map<const char*, System*> m_Systems{};
  };
}