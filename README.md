# TLM Analyzer

TLM Analyzer is a specialized tool for analyzing Transmission Line Model (TLM) data from CSV files. It calculates key semiconductor parameters including sheet resistance (Rsh), contact resistance (Rc), and specific contact resistivity (ρc).

## Features

- **CSV Data Processing**: Automatically processes TLM measurement data from CSV files
- **TLM Analysis**: Calculates sheet resistance, contact resistance, and specific contact resistivity
- **Data Visualization**: Interactive charts showing resistance vs. pad spacing with linear fit
- **Data Point Management**: Add or remove data points to refine analysis
- **Parameter Display**: Real-time display of calculated TLM parameters
- **Export Functionality**: Save analysis plots as PNG images

## Modular Architecture

The application is organized into the following modules:

1. **DataPoint Module**: Defines the data structure for TLM measurements
2. **CSV Processor Module**: Handles reading and parsing of CSV files
3. **Calculator Module**: Performs mathematical calculations and TLM analysis
4. **Data Manager Module**: Manages collections of data points
5. **UI/Plotting Module**: Handles user interface and data visualization

For detailed information about the architecture, see [ARCHITECTURE.md](ARCHITECTURE.md).

## Installation

### Prerequisites

- Qt 6.10.0 or later
- CMake 3.13 or later
- C++17 compatible compiler (MSVC 2022 recommended on Windows)

### Building

1. Clone or download the repository
2. Open the project in Qt Creator or your preferred IDE
3. Configure the project with CMake
4. Build the project

Alternatively, you can build from the command line:

```bash
mkdir build
cd build
cmake ..
make
```

On Windows with MSVC:
```cmd
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build .
```

## Usage

1. Launch the TLM Analyzer application
2. Click "Browse..." to select a folder containing your CSV measurement files
3. Adjust the resistance voltage if needed (default is 1.0V)
4. Click "Analyze Data" to process the files and perform TLM analysis
5. View the results in the chart and results panel
6. Optionally, add or remove data points to refine the analysis
7. Export the plot using the "Export Plot" button

### CSV File Format

The application expects CSV files with the following format:
- Column 6: Voltage (V)
- Column 7: Current (A)

Filenames should contain the pad spacing information that can be extracted with a regular expression.

## Technical Details

### TLM Parameter Calculations

The application performs linear regression on the resistance vs. pad spacing data to calculate:

- **Sheet Resistance (Rsh)**: Rsh = slope × 100 (Ω/sq)
- **Contact Resistance (Rc)**: Rc = intercept / 20 (Ω·mm)
- **Specific Contact Resistivity (ρc)**: ρc = (Rc² / Rsh) × 10⁻² (Ω·cm²)

### Linear Regression

The application uses a numerically stable linear regression algorithm based on centered data to ensure accuracy.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with Qt 6 Framework and Qt Charts Module
- Special thanks to Pudd1ng for support and contributions
- Inspired by the need for specialized TLM analysis tools in semiconductor research

## Author

JeongYeham - Initial work and implementation

For any questions or issues, please open an issue on the GitHub repository.