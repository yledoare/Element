/*
    AssetType.h - This file is part of Element
    Copyright (C) 2014  Kushview, LLC.  All rights reserved.
*/

#ifndef ELEMENT_ASSET_TYPE_H
#define ELEMENT_ASSET_TYPE_H


class AssetType
{
public:

    /** Numeric symbol for this asset type. */
    enum ID
    {
        AudioFile  = 0, // Regular audio
        MidiFile   = 1, // Regular MIDI
        Sequence   = 2, // Element Sequence Asset
        Video      = 3, // Video (not supported yet)
        Unknown    = 4
    };

    static const uint32_t numTypes = 4;
    inline AssetType() : root (Unknown) { }

    inline AssetType (const AssetType& other)
    {
        root = other.root;
    }

    AssetType (const ID root)
        : root (root)
    { }

    AssetType (const String& str)
        : root (Unknown)
    {
        if (str == "audio")
            root = AudioFile;
        if (str == "midi")
            root = MidiFile;
        if (str == "sequence")
            root = Sequence;
        else if (str == "video")
            root = Video;
    }

    /** Inverse of the from-string constructor */
    const char* toString() const
    {
        switch (root)
        {
            case AudioFile: return "audio";
            case MidiFile:  return "midi";
            case Sequence:  return "sequence";
            case Video:     return "video";
            default:        return "unknown"; // reeeally shouldn't ever happen
        }
    }

    const char* toURI() const
    {
        switch (root)
        {
            case AudioFile:    return "urn:datatype:audio";
            case MidiFile:     return "urn:datatype:midi";
            case Sequence: return "urn:datatype:sequence";
            case Video:    return "urn:datatype:video";
            default:       return "unknown"; // reeeally shouldn't ever happen
        }
    }

    inline operator uint32_t() const { return (uint32_t) root; }

    /** AssetType iterator */
    class iterator {
    public:

        iterator (uint32_t index) : index(index) { }

        AssetType  operator*() { return AssetType((ID)index); }
        iterator& operator++() { ++index; return *this; } // yes, prefix only
        bool operator==(const iterator& other) { return (index == other.index); }
        bool operator!=(const iterator& other) { return (index != other.index); }

    private:

        friend class AssetType;
        uint32_t index;

    };

    static iterator begin() { return iterator (0); }
    static iterator end()   { return iterator (numTypes); }

    bool operator==(const ID symbol) { return (root == symbol); }
    bool operator!=(const ID symbol) { return (root != symbol); }

    bool operator==(const AssetType other) { return (root == other.root); }
    bool operator!=(const AssetType other) { return (root != other.root); }

    AssetType& operator= (const AssetType other) { root = other.root; return *this; }

private:

    ID root; // could be const if not for the string constructor

};


#endif // ASSETTYPE_H