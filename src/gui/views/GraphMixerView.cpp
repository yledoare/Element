
#include "controllers/AppController.h"
#include "gui/NodeChannelStripComponent.h"
#include "gui/views/GraphMixerView.h"
#include "gui/views/GraphMixerView.h"
#include "gui/widgets/HorizontalListBox.h"
#include "gui/LookAndFeel.h"
#include "session/Session.h"
#include "Globals.h"
namespace Element {

class GraphMixerChannelStrip : public NodeChannelStripComponent,
                               public DragAndDropTarget
{
public:
std::function<void()> onReordered;

GraphMixerChannelStrip (GuiController& gui) : NodeChannelStripComponent (gui, false)
{
    onNodeChanged = [this]() { setNodeNameEditable (! (getNode().isIONode())); };
}

~GraphMixerChannelStrip() { }

void mouseDown (const MouseEvent& ev) override
{
    down = true; 
    dragging = false;
}

void mouseDrag (const MouseEvent& ev) override
{
    if (down && ! dragging)
    {
        dragging = true;
        auto* dnd = findParentComponentOfClass<DragAndDropContainer>();
        Image image (Image::ARGB, 1, 1, true);
        dnd->startDragging (var("graphMixerStrip"), this, image);
    }
}

void mouseMove (const MouseEvent& ev) override
{
    // noop
}

void mouseUp (const MouseEvent& ev) override
{
    dragging = down = hover = false;
}

bool shouldDrawDragImageWhenOver() override
{
    return false;
}

void itemDragEnter (const SourceDetails&) override
{
    hover = true;
    repaint();
}

void itemDragExit (const SourceDetails&) override
{
    hover = false;
    repaint();
}

void paintOverChildren (Graphics& g) override
{
    if (hover && ! dragging && ! down) {
        g.setColour (Colors::toggleBlue);
        g.drawRect(0, 0, getWidth(), getHeight(), 1);
    }
}

bool isInterestedInDragSource (const SourceDetails& details) override
{
   return details.description == "graphMixerStrip";
}

void itemDropped (const SourceDetails& details) override
{
    if (details.description == "graphMixerStrip")
    {
        auto* strip = dynamic_cast<GraphMixerChannelStrip*> (details.sourceComponent.get());
        auto myNode = getNode().getValueTree();
        auto dNode  = strip->getNode().getValueTree();
        ValueTree parent = dNode.getParent();

        int myIndex = parent.indexOf (myNode);
        int dIndex  = parent.indexOf (dNode);
        if (myIndex >= 0 && dIndex >= 0)
        {
            parent.moveChild (dIndex, myIndex, nullptr);
            if (onReordered)
                onReordered();
        }    
    }

    hover = false;
    repaint();
}

private:
    bool dragging = false;
    bool down = false;
    bool hover = false;

#if 0
    virtual void itemDragEnter (const SourceDetails& dragSourceDetails);
    virtual void itemDragMove (const SourceDetails& dragSourceDetails);
    virtual void itemDragExit (const SourceDetails& dragSourceDetails);
#endif
};

class GraphMixerListBoxModel : public ListBoxModel
{
public:
    GraphMixerListBoxModel (GuiController& g, HorizontalListBox& b) : gui (g), box(b) { refreshNodes(); }
    ~GraphMixerListBoxModel() { }

    int getNumRows() override
    {
        return nodes.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height,
                           bool rowIsSelected) override
    {
        ignoreUnused (rowNumber, g, width, height, rowIsSelected);
    }

    Node getNode (int r)
    {
        return nodes [r];
    }

    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, 
                                       Component* existing) override
    {
        GraphMixerChannelStrip* const strip = existing == nullptr
            ? new GraphMixerChannelStrip (gui) 
            : dynamic_cast<GraphMixerChannelStrip*> (existing);
        strip->onReordered = std::bind(&GraphMixerListBoxModel::onReordered, this);
        strip->setNode (getNode (rowNumber));
        return strip;
    }

    void onReordered() {
        refreshNodes();
        box.updateContent();
    }

    void refreshNodes()
    {
        nodes.clearQuick();
        const auto graph = gui.getWorld().getSession()->getActiveGraph();
        for (int i = 0; i < graph.getNumNodes(); ++i)
        {
            const auto node = graph.getNode (i);
            if (node.isMidiIONode())
                continue;
            nodes.add (node);
        }
    }
   #if 0
    virtual void listBoxItemClicked (int row, const MouseEvent&);
    virtual void listBoxItemDoubleClicked (int row, const MouseEvent&);
    virtual void backgroundClicked (const MouseEvent&);
    virtual void selectedRowsChanged (int lastRowSelected);
    virtual void deleteKeyPressed (int lastRowSelected);
    virtual void returnKeyPressed (int lastRowSelected);
    virtual void listWasScrolled();
    virtual var getDragSourceDescription (const SparseSet<int>& rowsToDescribe);
    virtual String getTooltipForRow (int row);
    virtual MouseCursor getMouseCursorForRow (int row);
   #endif
private:
    GuiController& gui;
    HorizontalListBox& box;
    NodeArray nodes;
    bool dragging = false;
};

class GraphMixerView::Content : public Component, public DragAndDropContainer
{
public:
    Content (GraphMixerView& v, GuiController& gui, Session* sess)
        : session (sess), view (v)
    {
        setOpaque (true);
        addAndMakeVisible (box);
        box.setRowHeight (80);
        model.reset (new GraphMixerListBoxModel (gui, box));
        box.setModel (model.get());
        box.updateContent();
    }

    ~Content()
    {
        box.setModel (nullptr);
        model.reset();
    }

    void resized() override
    {
        box.setBounds (getLocalBounds());
    }

    void paint (Graphics& g) override
    {
        g.setColour (LookAndFeel::widgetBackgroundColor.darker());
        g.fillAll();

        if (model->getNumRows() <= 0)
        {
            g.setColour (LookAndFeel::textColor);
            g.setFont (Font (15.f));
            g.drawText (TRANS ("No channels to display"), 
                getLocalBounds().toFloat(), Justification::centred);
        }
    }

    void stabilize()
    {
        model->refreshNodes();
        box.updateContent();
        repaint();
    }

private:
    SessionPtr session;
    GraphMixerView& view;
    std::unique_ptr<GraphMixerListBoxModel> model;
    ChannelStripComponent channelStrip;
    HorizontalListBox box;
};

GraphMixerView::GraphMixerView()
{
    setName (EL_VIEW_GRAPH_MIXER);
}

GraphMixerView::~GraphMixerView()
{
    content = nullptr;
}

void GraphMixerView::resized()
{
    if (content)
        content->setBounds (getLocalBounds());
}

void GraphMixerView::stabilizeContent()
{
    disableIfNotUnlocked();
    if (content)
        content->stabilize();
}

void GraphMixerView::initializeView (AppController& app)
{
    content.reset (new Content (*this, *app.findChild<GuiController>(),
                                app.getGlobals().getSession()));
    addAndMakeVisible (content.get());
    content->stabilize();
}

}