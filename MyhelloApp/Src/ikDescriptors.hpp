#pragma once
#ifndef IKDESCRIPTORS_HPP
#define IKDESCRIPTORS_HPP

#include "ikDeviceEngine.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace ikE {

    class IkDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(IkeDeviceEngine& ikDevice) : ikDevice{ ikDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<IkDescriptorSetLayout> build() const;

        private:
            IkeDeviceEngine& ikDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        IkDescriptorSetLayout(
            IkeDeviceEngine& ikDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~IkDescriptorSetLayout();
        IkDescriptorSetLayout(const IkDescriptorSetLayout&) = delete;
        IkDescriptorSetLayout& operator=(const IkDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        IkeDeviceEngine& ikDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class IkDescriptorWriter;
    };

    class IkDescriptorPool {
    public:
        class Builder {
        public:
            Builder(IkeDeviceEngine& ikDevice) : ikDevice{ ikDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<IkDescriptorPool> build() const;

        private:
            IkeDeviceEngine& ikDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        IkDescriptorPool(
            IkeDeviceEngine& ikDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~IkDescriptorPool();
        IkDescriptorPool(const IkDescriptorPool&) = delete;
        IkDescriptorPool& operator=(const IkDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        IkeDeviceEngine& ikDevice;
        VkDescriptorPool descriptorPool;

        friend class IkDescriptorWriter;
    };

    class IkDescriptorWriter {
    public:
        IkDescriptorWriter(IkDescriptorSetLayout& setLayout, IkDescriptorPool& pool);

        IkDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        IkDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        IkDescriptorSetLayout& setLayout;
        IkDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve
#endif