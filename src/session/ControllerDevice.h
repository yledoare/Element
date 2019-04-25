/*
    ControllerDevice.h - This file is part of Element
    Copyright (C) 2018  Kushview, LLC.  All rights reserved.
*/

#pragma once

#include "ElementApp.h"
#include "session/Node.h"

#define EL_OBJECT_GETTER(a,b) inline const var& get##a () const { return objectData.getProperty(b); }
#define EL_OBJECT_SETTER(a,b) inline void set##a (const var& value) { objectData.setProperty (b, value, nullptr); }
#define EL_OBJECT_GETTER_AND_SETTER(a,b) EL_OBJECT_GETTER(a,b) EL_OBJECT_SETTER(a,b)

namespace Element {

class ControllerDevice : public ObjectModel
{
public:
    enum ControlToggleMode 
    {
        EqualsOrHigher = 0,
        Equals
    };

    class Control : public ObjectModel
    {
    public:
        explicit Control (const ValueTree& data = ValueTree()) 
            : ObjectModel (data) 
        {
            if (data.isValid())
                setMissingProperties();
        }

        Control (const String& name)
            : ObjectModel (Tags::control)
        {
            setName (name);
            setMissingProperties();
        }
        
        ~Control() noexcept { }

        bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::control); }

        EL_OBJECT_GETTER_AND_SETTER(Name, Tags::name);
        EL_OBJECT_GETTER(ControlType, Tags::type);

        MidiMessage getMappingData() const
        {
            jassertfalse; // don't use this !
            return getMidiMessage();
        }
        
        MidiMessage getMidiMessage() const
        {
            MidiMessage midi;
            
            if (isNoteEvent())
            {
                midi = MidiMessage::noteOn (1, getEventId(), (uint8) 64);
            }
            else if (isControllerEvent())
            {
                midi = MidiMessage::controllerEvent (1, getEventId(), 64);
            }

            return midi;
        }

        bool isNoteEvent() const        { return getProperty("eventType").toString() == "note"; }
        bool isControllerEvent() const  { return getProperty("eventType").toString() == "controller"; }
        int getEventId() const          { return (int)  getProperty ("eventId", 0); }
        
        int getToggleValue() const      { return (int)  getProperty ("toggleValue", 0); }
        Value getToggleValueObject()    { return getPropertyAsValue ("toggleValue"); }
        bool inverseToggle() const      { return (bool) getProperty ("inverseToggle", false); }
        Value getInverseToggleObject()  { return getPropertyAsValue ("inverseToggle"); }
        
        static ControllerDevice::ControlToggleMode getToggleMode (const String& str)
        {
            if (str == "eqorhi")
                return ControllerDevice::EqualsOrHigher;
            else if (str == "eq")
                return ControllerDevice::Equals;
            return ControllerDevice::EqualsOrHigher;
        }
        ControllerDevice::ControlToggleMode getToggleMode() const
        {
            return getToggleMode (getProperty("toggleMode").toString());
        }
        Value getToggleModeObject() { return getPropertyAsValue ("toggleMode"); }

        ControllerDevice getControllerDevice() const
        {
            const ControllerDevice device (objectData.getParent());
            return device;
        }

        String getUuidString() const { return objectData.getProperty(Tags::uuid).toString(); }

    private:
        MidiMessage getMappingDataLegacy() const
        {
            const var& data (objectData.getProperty (Tags::mappingData));
            if (const auto* block = data.getBinaryData())
                if (block->getSize() > 0)
                    return MidiMessage (block->getData(), (int) block->getSize());
            return MidiMessage();
        }

        void setMissingProperties()
        {
            stabilizePropertyString (Tags::name, "Control");
            stabilizePropertyString (Tags::uuid, Uuid().toString());
            
            if (hasProperty (Tags::mappingData))
            {
                const auto midi = getMappingDataLegacy();
                if (midi.isNoteOnOrOff())
                {
                    setProperty ("eventType", "note");
                    setProperty ("eventId", midi.getNoteNumber());
                }
                else if (midi.isController())
                {
                    setProperty ("eventType", "controller");
                    setProperty ("eventId", midi.getControllerNumber());
                }
                objectData.removeProperty (Tags::mappingData, nullptr);
            }

            stabilizePropertyString ("eventType", "controller");
            stabilizePropertyPOD ("eventId", 0);
            stabilizePropertyPOD (Tags::midiChannel, 0);
            stabilizePropertyPOD ("toggleValue", 64);
            stabilizePropertyPOD ("inverseToggle", false);
            stabilizePropertyString ("toggleMode", "eqorhi");
        }
    };

    explicit ControllerDevice (const ValueTree& data = ValueTree());
    ControllerDevice (const String& name);
    virtual ~ControllerDevice() { }

    inline bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::controller); }

    EL_OBJECT_GETTER_AND_SETTER(Name, Tags::name)
    EL_OBJECT_GETTER(InputDevice, "inputDevice")
    inline String getUuidString() const { return objectData.getProperty(Tags::uuid).toString(); }

    inline int getNumControls() const { return getNumChildren(); }
    inline Control getControl (const int index) const
    {
        const auto child (objectData.getChild (index));
        return Control (child);
    }

    inline int indexOf (const ObjectModel& model) const { return objectData.indexOf (model.getValueTree()); }
    inline int indexOf (const Identifier& childType, const ObjectModel& model) const {
        return objectData.getChildWithName (childType).indexOf (model.getValueTree());
    }

    inline Control findControlById (const Uuid& uuid) const
    {
        const Control control (objectData.getChildWithProperty (Tags::uuid, uuid.toString()));
        return control;
    }

private:
    void setMissingProperties();
};

class ControllerMap : public ObjectModel
{
public:

    explicit ControllerMap (const ValueTree& data = ValueTree()) : ObjectModel (data) { }
    ~ControllerMap() noexcept { }
    inline bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::map); }
    inline int getParameterIndex() const  { return (int) objectData.getProperty (Tags::parameter, -1); }
};

}
