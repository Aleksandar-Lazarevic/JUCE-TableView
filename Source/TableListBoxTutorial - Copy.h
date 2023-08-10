/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             TableListBoxTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Explores table list boxes.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

//==============================================================================
//! [TableTutorialComponent]
class TableTutorialComponent    : public juce::Component,
                                  public juce::TableListBoxModel
{
//! [TableTutorialComponent]
public:
//! [TableTutorialComponent constructor preamble]
    TableTutorialComponent()
    {
        const auto callback = [this] (const juce::FileChooser& chooser)
        {
            loadData (chooser.getResult());                                             // [1]
//! [TableTutorialComponent constructor preamble]

//! [TableTutorialComponent constructor table]
            addAndMakeVisible (table);                                                  // [1]

            table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);      // [2]
            table.setOutlineThickness (1);
//! [TableTutorialComponent constructor table]

//! [TableTutorialComponent constructor addColumn]
            if (columnList != nullptr)
            {
                for (auto* columnXml : columnList->getChildIterator())
                {
                    table.getHeader().addColumn (columnXml->getStringAttribute ("name"), // [2]
                                                 columnXml->getIntAttribute ("columnId"),
                                                 columnXml->getIntAttribute ("width"),
                                                 50,
                                                 400,
                                                 juce::TableHeaderComponent::defaultFlags);
                }
            }
//! [TableTutorialComponent constructor addColumn]

//! [TableTutorialComponent constructor table setup]
            table.getHeader().setSortColumnId (1, true);                                // [3]

            table.setMultipleSelectionEnabled (true);                                   // [4]
//! [TableTutorialComponent constructor table setup]

            resized();
        };

        fileChooser.launchAsync (  juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
                                 callback);
    }

//! [getNumRows]
    int getNumRows() override
    {
        return numRows;
    }
//! [getNumRows]

//! [paintRowBackground]
    void paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
                                               .interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll (juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
    }
//! [paintRowBackground]

//! [paintCell]
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
//! [paintCell]

//! [sortOrderChanged]
    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            TutorialDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            table.updateContent();
        }
    }
//! [sortOrderChanged]

//! [refreshComponentForCell]
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
//! [refreshComponentForCell]

//! [getColumnAutoSizeWidth]
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
//! [getColumnAutoSizeWidth]

//! [getSelection]
    int getSelection (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Select");
    }
//! [getSelection]

//! [setSelection]
    void setSelection (const int rowNumber, const int newSelection)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Select", newSelection);
    }
//! [setSelection]

//! [getText]
    juce::String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId (columnNumber));
    }
//! [getText]

//! [setText]
    void setText (const int columnNumber, const int rowNumber, const juce::String& newText)
    {
        const auto& columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }
//! [setText]

    //==============================================================================
    void resized() override
    {
        table.setBoundsInset (juce::BorderSize<int> (8));
    }

//! [TableTutorialComponent allMembers]
private:
    juce::TableListBox table  { {}, this };
    juce::Font font           { 14.0f };

//! [xml members]
    std::unique_ptr<juce::XmlElement> tutorialData;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
//! [xml members]
    int numRows = 0;
//! [TableTutorialComponent allMembers]

    //==============================================================================
//! [EditableTextCustomComponent]
    class EditableTextCustomComponent  : public juce::Label
    {
    public:
        EditableTextCustomComponent (TableTutorialComponent& td)
            : owner (td)
        {
            setEditable (false, true, false);
        }
//! [EditableTextCustomComponent]

//! [EditableTextCustomComponent mouseDown]
        void mouseDown (const juce::MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }
//! [EditableTextCustomComponent mouseDown]

//! [EditableTextCustomComponent textWasEdited]
        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }
//! [EditableTextCustomComponent textWasEdited]

//! [EditableTextCustomComponent setRowAndColumn]
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), juce::dontSendNotification);
        }
//! [EditableTextCustomComponent setRowAndColumn]

//! [EditableTextCustomComponent members]
    private:
        TableTutorialComponent& owner;
        int row, columnId;
        juce::Colour textColour;
    };
//! [EditableTextCustomComponent members]

    //==============================================================================
//! [SelectionColumnCustomComponent]
    class SelectionColumnCustomComponent    : public Component
    {
    public:
        SelectionColumnCustomComponent (TableTutorialComponent& td)
            : owner (td)
        {
            addAndMakeVisible (toggleButton);

            toggleButton.onClick = [this] { owner.setSelection (row, (int) toggleButton.getToggleState()); };
        }
//! [SelectionColumnCustomComponent]

//! [SelectionColumnCustomComponent resized]
        void resized() override
        {
            toggleButton.setBoundsInset (juce::BorderSize<int> (2));
        }
//! [SelectionColumnCustomComponent resized]

//! [SelectionColumnCustomComponent setRowAndColumn]
        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState ((bool) owner.getSelection (row), juce::dontSendNotification);
        }
//! [SelectionColumnCustomComponent setRowAndColumn]

//! [SelectionColumnCustomComponent members]
    private:
        TableTutorialComponent& owner;
        juce::ToggleButton toggleButton;
        int row, columnId;
    };
//! [SelectionColumnCustomComponent members]

    //==============================================================================
//! [TutorialDataSorter]
    class TutorialDataSorter
    {
    public:
        TutorialDataSorter (const juce::String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {}
//! [TutorialDataSorter]

//! [TutorialDataSorter compareElements]
        int compareElements (juce::XmlElement* first, juce::XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                                .compareNatural (second->getStringAttribute (attributeToSort)); // [1]

            if (result == 0)
                result = first->getStringAttribute ("ID")
                               .compareNatural (second->getStringAttribute ("ID"));             // [2]

            return direction * result;                                                          // [3]
        }
//! [TutorialDataSorter compareElements]

//! [TutorialDataSorter members]
    private:
        juce::String attributeToSort;
        int direction;
    };
//! [TutorialDataSorter members]

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableTutorialComponent)
};

//==============================================================================
class MainComponent   : public juce::Component
{
public:
    //==============================================================================
    MainComponent()
    {
        addAndMakeVisible (table);

        setSize (1200, 600);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        table.setBounds (getLocalBounds());
    }

private:
    //==============================================================================
    TableTutorialComponent table;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
