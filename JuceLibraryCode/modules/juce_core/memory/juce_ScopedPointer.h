/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    This class is deprecated. You should use std::unique_ptr instead.


    A ScopedPointer holds a pointer that is automatically deleted when the ScopedPointer
    goes out of scope.

    Once a pointer has been passed to a ScopedPointer, it will make sure that the pointer
    gets deleted when the ScopedPointer is deleted. Using the ScopedPointer on the stack or
    as member variables is a good way to use RAII to avoid accidentally leaking dynamically
    created objects.

    A ScopedPointer can be used in pretty much the same way that you'd use a normal pointer
    to an object. If you use the assignment operator to assign a different object to a
    ScopedPointer, the old one will be automatically deleted.

    Important note: The class is designed to hold a pointer to an object, NOT to an array!
    It calls delete on its payload, not delete[], so do not give it an array to hold! For
    that kind of purpose, you should be using HeapBlock or Array instead.

    A const ScopedPointer is guaranteed not to lose ownership of its object or change the
    object to which it points during its lifetime. This means that making a copy of a const
    ScopedPointer is impossible, as that would involve the new copy taking ownership from the
    old one.

    If you need to get a pointer out of a ScopedPointer without it being deleted, you
    can use the release() method.

    @tags{Core}
*/
template <class ObjectType>
class ScopedPointer
{
public:
    //==============================================================================
    /** Creates a ScopedPointer containing a null pointer. */
    inline ScopedPointer() noexcept {}

    /** Creates a ScopedPointer containing a null pointer. */
    inline ScopedPointer (decltype (nullptr)) noexcept {}

    /** Creates a ScopedPointer that owns the specified object. */
    inline ScopedPointer (ObjectType* objectToTakePossessionOf) noexcept
        : object (objectToTakePossessionOf)
    {
    }

    /** Creates a ScopedPointer that takes its pointer from another ScopedPointer.

        Because a pointer can only belong to one ScopedPointer, this transfers
        the pointer from the other object to this one, and the other object is reset to
        be a null pointer.
    */
    ScopedPointer (ScopedPointer& objectToTransferFrom) noexcept
        : object (objectToTransferFrom.release())
    {
    }

    /** Destructor.
        If the ScopedPointer currently refers to an object, it'll be deleted.
    */
    inline ~ScopedPointer()         { reset(); }

    /** Changes this ScopedPointer to point to a new object.

        Because a pointer can only belong to one ScopedPointer, this transfers
        the pointer from the other object to this one, and the other object is reset to
        be a null pointer.

        If this ScopedPointer already points to an object, that object
        will first be deleted.
    */
    ScopedPointer& operator= (ScopedPointer& objectToTransferFrom)
    {
        if (this != objectToTransferFrom.getAddress())
        {
            // Two ScopedPointers should never be able to refer to the same object - if
            // this happens, you must have done something dodgy!
            jassert (object == nullptr || object != objectToTransferFrom.object);
            reset (objectToTransferFrom.release());
        }

        return *this;
    }

    /** Changes this ScopedPointer to point to a new object.
        If this ScopedPointer already points to an object, that object will first be deleted.
        The pointer that you pass in may be a nullptr.
    */
    ScopedPointer& operator= (ObjectType* newObjectToTakePossessionOf)
    {
        reset (newObjectToTakePossessionOf);
        return *this;
    }

    /** Take ownership of another ScopedPointer */
    ScopedPointer (ScopedPointer&& other) noexcept  : object (other.object)
    {
        other.object = nullptr;
    }

    /** Take ownership of another ScopedPointer */
    ScopedPointer& operator= (ScopedPointer&& other) noexcept
    {
        reset (other.release());
        return *this;
    }

    //==============================================================================
    /** Returns the object that this ScopedPointer refers to. */
    inline operator ObjectType*() const noexcept                                    { return object; }

    /** Returns the object that this ScopedPointer refers to. */
    inline ObjectType* get() const noexcept                                         { return object; }

    /** Returns the object that this ScopedPointer refers to. */
    inline ObjectType& operator*() const noexcept                                   { return *object; }

    /** Lets you access methods and properties of the object that this ScopedPointer refers to. */
    inline ObjectType* operator->() const noexcept                                  { return object; }

    //==============================================================================
    /** Clears this pointer, deleting the object it points to if there is one. */
    void reset()
    {
        ContainerDeletePolicy<ObjectType>::destroy (object);
        object = {};
    }

    /** Sets this pointer to a new object, deleting the old object that it was previously pointing to if there was one. */
    void reset (ObjectType* newObject)
    {
        if (object != newObject)
        {
            auto* oldObject = object;
            object = newObject;
            ContainerDeletePolicy<ObjectType>::destroy (oldObject);
        }
    }

    /** Sets this pointer to a new object, deleting the old object that it was previously pointing to if there was one. */
    void reset (ScopedPointer& newObject)
    {
        reset (newObject.release());
    }

    /** Detaches and returns the current object from this ScopedPointer without deleting it.
        This will return the current object, and set the ScopedPointer to a null pointer.
    */
    ObjectType* release() noexcept                                                  { auto* o = object; object = {}; return o; }

    //==============================================================================
    /** Swaps this object with that of another ScopedPointer.
        The two objects simply exchange their pointers.
    */
    void swapWith (ScopedPointer<ObjectType>& other) noexcept
    {
        // Two ScopedPointers should never be able to refer to the same object - if
        // this happens, you must have done something dodgy!
        jassert (object != other.object || this == other.getAddress() || object == nullptr);

        std::swap (object, other.object);
    }

    /** If the pointer is non-null, this will attempt to return a new copy of the object that is pointed to.
        If the pointer is null, this will safely return a nullptr.
    */
    inline ObjectType* createCopy() const                                           { return createCopyIfNotNull (object); }

private:
    //==============================================================================
    ObjectType* object = nullptr;

    const ScopedPointer* getAddress() const noexcept  { return this; } // Used internally to avoid the & operator

   #if ! JUCE_MSVC  // (MSVC can't deal with multiple copy constructors)
    ScopedPointer (const ScopedPointer&) = delete;
    ScopedPointer& operator= (const ScopedPointer&) = delete;
   #endif
};

//==============================================================================
/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator== (ObjectType1* pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1 == pointer2.get();
}

/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator!= (ObjectType1* pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1 != pointer2.get();
}

/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator== (const ScopedPointer<ObjectType1>& pointer1, ObjectType2* pointer2) noexcept
{
    return pointer1.get() == pointer2;
}

/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator!= (const ScopedPointer<ObjectType1>& pointer1, ObjectType2* pointer2) noexcept
{
    return pointer1.get() != pointer2;
}

/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator== (const ScopedPointer<ObjectType1>& pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1.get() == pointer2.get();
}

/** Compares a ScopedPointer with another pointer. */
template <typename ObjectType1, typename ObjectType2>
bool operator!= (const ScopedPointer<ObjectType1>& pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1.get() != pointer2.get();
}

/** Compares a ScopedPointer with a nullptr. */
template <class ObjectType>
bool operator== (decltype (nullptr), const ScopedPointer<ObjectType>& pointer) noexcept
{
    return pointer.get() == nullptr;
}

/** Compares a ScopedPointer with a nullptr. */
template <class ObjectType>
bool operator!= (decltype (nullptr), const ScopedPointer<ObjectType>& pointer) noexcept
{
    return pointer.get() != nullptr;
}

/** Compares a ScopedPointer with a nullptr. */
template <class ObjectType>
bool operator== (const ScopedPointer<ObjectType>& pointer, decltype (nullptr)) noexcept
{
    return pointer.get() == nullptr;
}

/** Compares a ScopedPointer with a nullptr. */
template <class ObjectType>
bool operator!= (const ScopedPointer<ObjectType>& pointer, decltype (nullptr)) noexcept
{
    return pointer.get() != nullptr;
}

//==============================================================================
#ifndef DOXYGEN
// NB: This is just here to prevent any silly attempts to call deleteAndZero() on a ScopedPointer.
template <typename Type>
void deleteAndZero (ScopedPointer<Type>&)  { static_assert (sizeof (Type) == 12345,
                                                            "Attempt to call deleteAndZero() on a ScopedPointer"); }
#endif

} // namespace juce
