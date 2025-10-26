# TLM Analyzer Modular Architecture

## Overview

This document describes the modular architecture of the TLM Analyzer application, which has been refactored to improve maintainability and separation of concerns.

## Modules

### 1. DataPoint Module (`datapoint.h`)
- **Purpose**: Defines the data structure for TLM measurement data points
- **Key Components**:
  - `DataPoint` struct containing spacing, resistance, current, and enabled status
  - Qt meta-type declarations for use in signals/slots

### 2. CSV Processor Module (`csvprocessor.h`, `csvprocessor.cpp`)
- **Purpose**: Handles reading and parsing of CSV files containing measurement data
- **Key Components**:
  - `CSVProcessor` class with static methods
  - File and folder processing functions
  - Voltage and current extraction from CSV data
  - Spacing extraction from filenames

### 3. Calculator Module (`calculator.h`, `calculator.cpp`)
- **Purpose**: Performs mathematical calculations and TLM analysis
- **Key Components**:
  - `Calculator` class with static methods
  - Linear regression implementation
  - TLM parameter calculations (sheet resistance, contact resistance, etc.)
  - `TLMResult` struct for calculation results

### 4. Data Manager Module (`datamanager.h`, `datamanager.cpp`)
- **Purpose**: Manages collections of data points and provides data manipulation capabilities
- **Key Components**:
  - `DataManager` class
  - Data point addition, removal, and modification
  - Enabled/disabled state management
  - TLM result calculation coordination
  - Qt signals for data change notifications

### 5. UI/Plotting Module (`mainwindow.h`, `mainwindow.cpp`)
- **Purpose**: Handles user interface, visualization, and user interactions
- **Key Components**:
  - `MainWindow` class
  - Qt Charts integration for data visualization
  - User input handling
  - Result display

### 6. Legacy Analyzer Module (`tlmanalyzer.h`, `tlmanalyzer.cpp`)
- **Purpose**: Original analyzer implementation (to be deprecated in favor of modular approach)
- **Key Components**:
  - `TLMAnalyzer` class
  - Folder analysis functionality
  - Signal/slot based communication

## Data Flow

1. **Data Input**: CSV files are processed by the CSV Processor Module
2. **Data Storage**: Processed data is managed by the Data Manager Module
3. **Calculations**: Calculator Module performs TLM analysis on the data
4. **Visualization**: UI/Plotting Module displays results and charts
5. **Interaction**: User interactions modify data through the Data Manager Module

## Benefits of Modular Design

1. **Separation of Concerns**: Each module has a specific, well-defined responsibility
2. **Maintainability**: Changes to one module have minimal impact on others
3. **Testability**: Individual modules can be tested in isolation
4. **Extensibility**: New features can be added with minimal disruption
5. **Reusability**: Modules can potentially be reused in other applications