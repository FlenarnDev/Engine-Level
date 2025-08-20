# Project Name

Fallout Cascadia

## Introduction

This project is a re-creation of remagination of the Fallout series based in the Greater Seatte Area. We use the Fallout 4 Script Extender (F4SE). 
## Getting Started

### Prerequisites

- Fallout 4 Script Extender (F4SE)
- A compatible version of Fallout 4 (Latest version, .984)
- Address Library for release .984

### Installation

1. Download the latest release of the plugin.
2. Extract the contents of the downloaded file into your Fallout 4 directory.

## Contributing

We welcome contributions from everyone. Here are a few guidelines to help you get started:

1. **Fork the Repository**: Start by forking this repository to your GitHub account.

2. **Clone the Repository**: After forking, clone the repository to your local machine.

3. **Create a New Branch**: Always create a new branch for your changes. This keeps the main branch clean and makes it easier to manage your changes.

4. **Make Your Changes**: Make your changes in the new branch. Please follow the existing coding style.

5. **Test Your Changes**: Before submitting your changes, make sure to test them thoroughly.

6. **Submit a Pull Request**: Once you're satisfied with your changes, push them to your forked repository and submit a pull request. We'll review your changes and merge them if everything checks out.

## Understanding the Code

The main entry point of the plugin is the `F4SEPlugin_Load` function. This function initializes the plugin, registers the messaging and scaleform interfaces, and installs the dialogue menu, patches, and scripts.

The `F4SE::AllocTrampoline` function is used to allocate memory for the plugin.

The `RE::Cascadia::CAS_DialogueMenu::Install` function installs the dialogue menu.

The `RE::Cascadia::Patches::Install` function installs the patches.

The `ObScript::Install` function installs the scripts.

## License

This project is licensed under the GPL-3.0 License. See the LICENSE file for more details.

## Contact

If you have any questions, issues, or suggestions, please open an issue on GitHub. We'll do our best to help you out.
