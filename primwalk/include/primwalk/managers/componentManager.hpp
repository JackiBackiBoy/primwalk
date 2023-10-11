#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/componentArray.hpp"
#include "primwalk/components/component.hpp"

// std
#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace pw {
  class PW_API ComponentManager {
  public:
    // Remove copy/move constructors
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager(ComponentManager&&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;
    ComponentManager& operator=(ComponentManager&&) = delete;

    static ComponentManager& Get();

    template<typename T>
    void registerComponent() {
      const char* typeName = typeid(T).name();

      assert(m_ComponentTypes.find(typeName) == m_ComponentTypes.end() && "ERROR: Component type already registered!");
      
      m_ComponentTypes.insert({ typeName, m_NextComponentType });
      m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

      m_NextComponentType++;
    }

    template<typename T>
    component_type getComponentType() {
      const char* typeName = typeid(T).name();

      assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered before use");

      return m_ComponentTypes[typeName];
    }

    template<typename T, typename... Args>
    T& addComponent(entity_id entity, Args&&... args) {
      getComponentArray<T>()->insert(entity, std::forward<Args>(args)...);
      return getComponent<T>(entity);
    }

    template<typename T>
    void removeComponent(entity_id entity) {
      getComponentArray<T>()->remove(entity);
    }

    template<typename T>
    T& getComponent(entity_id entity) {
      return getComponentArray<T>()->get(entity);
    }

    void entityDestroyed(entity_id entity);

  private:
    ComponentManager() = default;
    ~ComponentManager() = default;

    std::unordered_map<const char*, component_type> m_ComponentTypes{};
    std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays{};
    component_type m_NextComponentType{};

    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
      const char* typeName = typeid(T).name();

      assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "ERROR: Component not registered before use!");
      
      return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
    }
  };
}