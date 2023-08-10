#pragma once
#include <JuceHeader.h>
//==============================================================================
class PropertyWndComponent    : public juce::Component,
                                  public juce::TableListBoxModel
{
public:
    PropertyWndComponent()
    {
#if 1
        juce::String strPath = "TableData.xml";
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile(strPath);
        loadData(f);                                             // [1]

        addAndMakeVisible(tlbObject);                                                  // [1]

        tlbObject.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);      // [2]
        tlbObject.setOutlineThickness(1);
        if (columnList != nullptr)
        {
            for (auto* columnXml : columnList->getChildIterator())
            {
                tlbObject.getHeader().addColumn(columnXml->getStringAttribute("name"), // [2]
                    columnXml->getIntAttribute("columnId"),
                    columnXml->getIntAttribute("width"),
                    50,
                    400,
                    juce::TableHeaderComponent::defaultFlags);
            }
        }
        tlbObject.getHeader().setSortColumnId(1, true);                                // [3]

        tlbObject.setMultipleSelectionEnabled(true);                                   // [4]

        resized();
#else
        const auto callback = [this](const juce::FileChooser& chooser)
        {
            loadData(chooser.getResult());                                             // [1]

            addAndMakeVisible(table);                                                  // [1]

            table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);      // [2]
            table.setOutlineThickness(1);
            if (columnList != nullptr)
            {
                for (auto* columnXml : columnList->getChildIterator())
                {
                    table.getHeader().addColumn(columnXml->getStringAttribute("name"), // [2]
                        columnXml->getIntAttribute("columnId"),
                        columnXml->getIntAttribute("width"),
                        50,
                        400,
                        juce::TableHeaderComponent::defaultFlags);
                }
            }
            table.getHeader().setSortColumnId(1, true);                                // [3]

            table.setMultipleSelectionEnabled(true);                                   // [4]

            resized();
        };

        fileChooser.launchAsync(juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles,
            callback);
#endif // 0

        
        
    }
    ~PropertyWndComponent() {
        tlbObject.setModel(nullptr);
    }

    int getNumRows() override
    {
        return numRows;
    }

    void paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
                                               .interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll (juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
    }

    void paintCell (juce::Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour (juce::ListBox::textColourId));  // [5]
        g.setFont (font);

        if (auto* rowElement = dataList->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);                             // [6]
        }

        g.setColour (getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);                                                                               // [7]
    }

    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            tlbObject.updateContent();
        }
    }

    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId == 9)  // [8]
        {
            auto* selectionBox = static_cast<SelectionColumnCustomComponent*> (existingComponentToUpdate);

            if (selectionBox == nullptr)
                selectionBox = new SelectionColumnCustomComponent (*this);

            selectionBox->setRowAndColumn (rowNumber, columnId);
            return selectionBox;
        }

        if (columnId == 8)  // [9]
        {
            auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

            if (textLabel == nullptr)
                textLabel = new EditableTextCustomComponent (*this);

            textLabel->setRowAndColumn (rowNumber, columnId);
            return textLabel;
        }

        jassert (existingComponentToUpdate == nullptr);
        return nullptr;     // [10]
    }

    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 9)
            return 50;

        int widest = 32;

        for (auto i = getNumRows(); --i >= 0;)
        {
            if (auto* rowElement = dataList->getChildElement (i))
            {
                auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

                widest = juce::jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    int getSelection (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Select");
    }

    void setSelection (const int rowNumber, const int newSelection)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Select", newSelection);
    }

    juce::String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId (columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const juce::String& newText)
    {
        const auto& columnName = tlbObject.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    void resized() override
    {
        tlbObject.setBoundsInset (juce::BorderSize<int> (8));
    }

private:
    juce::TableListBox tlbObject  { {}, this };
    juce::Font font           { 14.0f };

    std::unique_ptr<juce::XmlElement> tutorialData;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
    int numRows = 0;

    class EditableTextCustomComponent  : public juce::Label
    {
    public:
        EditableTextCustomComponent (PropertyWndComponent& td)
            : owner (td)
        {
            setEditable (false, true, false);
        }

        void mouseDown (const juce::MouseEvent& event) override
        {
            owner.tlbObject.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), juce::dontSendNotification);
        }

    private:
        PropertyWndComponent& owner;
        int row, columnId;
        juce::Colour textColour;
    };

    class SelectionColumnCustomComponent    : public Component
    {
    public:
        SelectionColumnCustomComponent (PropertyWndComponent& td)
            : owner (td)
        {
            addAndMakeVisible (toggleButton);

            toggleButton.onClick = [this] { owner.setSelection (row, (int) toggleButton.getToggleState()); };
        }

        void resized() override
        {
            toggleButton.setBoundsInset (juce::BorderSize<int> (2));
        }

        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState ((bool) owner.getSelection (row), juce::dontSendNotification);
        }

    private:
        PropertyWndComponent& owner;
        juce::ToggleButton toggleButton;
        int row, columnId;
    };

    class DataSorter
    {
    public:
        DataSorter (const juce::String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {}

        int compareElements (juce::XmlElement* first, juce::XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                                .compareNatural (second->getStringAttribute (attributeToSort)); // [1]

            if (result == 0)
                result = first->getStringAttribute ("ID")
                               .compareNatural (second->getStringAttribute ("ID"));             // [2]

            return direction * result;                                                          // [3]
        }

    private:
        juce::String attributeToSort;
        int direction;
    };

    //==============================================================================
//! [loadData]
    void loadData (juce::File tableFile)
    {
        if (tableFile == juce::File() || ! tableFile.exists())
            return;

        tutorialData = juce::XmlDocument::parse (tableFile);            // [3]

        dataList   = tutorialData->getChildByName ("DATA");
        columnList = tutorialData->getChildByName ("HEADERS");          // [4]

        numRows = dataList->getNumChildElements();                      // [5]
    }
//! [loadData]

//! [getAttributeNameForColumnId]
    juce::String getAttributeNameForColumnId (const int columnId) const
    {
        for (auto* columnXml : columnList->getChildIterator())
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return {};
    }
//! [getAttributeNameForColumnId]

    juce::FileChooser fileChooser { "Browse for TableData.xml",
                                    juce::File::getSpecialLocation (juce::File::invokedExecutableFile) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyWndComponent)
};

