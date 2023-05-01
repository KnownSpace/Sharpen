#include <sharpen/FiberLocalStorage.hpp>

sharpen::FiberLocalStorage::FiberLocalStorage() noexcept
    : storage_(nullptr) {
}

sharpen::FiberLocalStorage &sharpen::FiberLocalStorage::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->storage_ = std::move(other.storage_);
    }
    return *this;
}

sharpen::FiberLocalStorage::~FiberLocalStorage() noexcept {
    std::unique_ptr<Map> storage{std::move(this->storage_)};
    if (storage) {
        for (auto begin = storage->begin(), end = storage->end(); begin != end; ++begin) {
            AnyPointer pointer{begin->second};
            Dtor dtor{nullptr};
            void *p{nullptr};
            std::swap(pointer.dtor_, dtor);
            std::swap(pointer.pointer_, p);
            if (dtor && p) {
                dtor(p);
            }
        }
    }
}

void *sharpen::FiberLocalStorage::Lookup(std::size_t index) const noexcept {
    if (this->storage_) {
        auto ite = this->storage_->find(index);
        if (ite != this->storage_->end()) {
            return ite->second.pointer_;
        }
    }
    return nullptr;
}

void sharpen::FiberLocalStorage::Put(std::size_t index, void *p, Dtor dtor) {
    if (!this->storage_) {
        this->storage_.reset(new Map{});
    }
    auto ite = this->storage_->find(index);
    if (ite != this->storage_->end()) {
        std::swap(ite->second.dtor_, dtor);
        std::swap(ite->second.pointer_, p);
        if (p && dtor) {
            dtor(p);
        }
    } else {
        AnyPointer pointer;
        pointer.dtor_ = dtor;
        pointer.pointer_ = p;
        this->storage_->emplace(index, pointer);
    }
}

void sharpen::FiberLocalStorage::Remove(std::size_t index) noexcept {
    if (this->storage_) {
        auto ite = this->storage_->find(index);
        if (ite != this->storage_->end()) {
            void *p{nullptr};
            Dtor dtor{nullptr};
            std::swap(p, ite->second.pointer_);
            std::swap(dtor, ite->second.dtor_);
            if (dtor && p) {
                dtor(p);
            }
        }
    }
}

void sharpen::FiberLocalStorage::Erase(std::size_t index) noexcept {
    if (this->storage_) {
        auto ite = this->storage_->find(index);
        if (ite != this->storage_->end()) {
            if (ite->second.dtor_ && ite->second.pointer_) {
                ite->second.dtor_(ite->second.pointer_);
            }
            this->storage_->erase(ite);
        }
    }
}

std::atomic_size_t sharpen::FiberLocalStorage::nextIndex_{0};

std::size_t sharpen::FiberLocalStorage::GetNextIndex() noexcept {
    return Self::nextIndex_.fetch_add(1, std::memory_order::memory_order_relaxed);
}