# Contributing to Generic Node Sensor Edition

Thank you for your interest in helping us build the Generic Node Sensor Edition (GNSE).

We provide the hardware and firmware of the device to support the LoRaWAN ecosystem and help build awesome LoRaWAN devices.
Any contribution to the project is appreciated.

There are many ways to contribute to this project.

## Types of contributions :memo:

### :mega: Discussions

We love to discuss design decisions, device features and possible use cases with the community, so please use the [forum](https://www.thethingsnetwork.org/forum/).

### :question: Questions

 If you have questions, please use the [forum](https://www.thethingsnetwork.org/forum/). We have a special [category for The Generic Node](https://www.thethingsnetwork.org/forum/c/nodes/generic-node/88).

### :bulb: New features

 If you have a great idea or think some functionality is missing, we want to know! The only thing you have to do for that is to [create a Feature request issue](https://github.com/TheThingsIndustries/generic-node-se/issues) if it doesn't exist yet. Please use the issue template and fill out all sections.


### :beetle: Issues

If you notice that one of the applications is not functioning properly or there may be a bug in the implementation, please [create a Bug report issue](https://github.com/TheThingsIndustries/generic-node-se/issues) if it doesn't exist yet. Please use the issue template and fill out all sections.

### :hammer_and_wrench: Pull requests

If you see an [open issue](https://github.com/TheThingsIndustries/generic-node-se/issues) that you would like to work on, let the other contributors know by commenting in the issue.

### :books: Documentation

If you see that our documentation is lacking or incorrect, it would be great if you could help us improve it. This will help users and fellow contributors understand how to better develop and use the device. Generic Node documentation [repository](https://github.com/TheThingsIndustries/generic-node-docs) can be viewed at [documentation website](https://www.genericnode.com/docs/) and you can contribute by following the [documentation contribution guideline](https://github.com/TheThingsIndustries/generic-node-docs/blob/master/CONTRIBUTING.md).

## Steps to contribute

- [Fork the repository](https://docs.github.com/en/github/getting-started-with-github/fork-a-repo#fork-an-example-repository)

- Follow our [getting started](https://www.genericnode.com/docs/getting-started/) guides to setup your environment

- Create a branch following our [branching guideline](#branching-guideline)

- Commit your changes following our [commit messages guideline](#commit-messages-guideline)

- Open a [pull request](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request)

- Project maintainers will review as soon as possible

## Branching guideline

### Naming

All branches shall have one of these names. There are no exceptions.

- `fix/#-short-name` or `fix/short-name`: refers to a fix, optionally with issue number
- `feature/#-short-name` or `feature/short-name`: feature branch, optionally with issue number
- `issue/#-short-name`: anything else that refers to an issue but is not clearly a fix nor a feature
- `develop`: is usually the default branch. This is a clean branch where reviewed and approved pull requests are merged into, and as a contributor your pull requests should target this branch.
- `master`: is the production branch, and contains deployment-ready code.

Only code in `master` should be deployed to production, while code in other branches may be deployed for testing purposes.

### Scope

A fix, feature or issue branch should be **small and focused** and should be scoped to a **single specific task**. Do not combine new features and refactoring of existing code.

### Rebasing, Merging

- `fix/`, `feature/`, `issue/` branches are merged into `develop`
  - These branches may be rebased on `develop` in order to clean up the commit history
- `develop` branch is merged into `master` later by the maintainers of the project
  - `develop` must not ever be rebased

[git rebase](https://git-scm.com/docs/git-rebase) is often used before merging a branch. If you are not familiar with rebasing, you should read [this](https://git-scm.com/book/en/v2/Git-Branching-Rebasing) guide. It is also used in order to [clean up](https://git-scm.com/book/en/v2/Git-Tools-Rewriting-History) your commit trail before merging to `develop`.

## Commit messages guideline

The first line of a commit message is the subject. The commit message may contain a body, separated from the subject by an empty line.

### Subject

The subject contains the concerning component or topic (if applicable) and a concise message in [the imperative mood](https://chris.beams.io/posts/git-commit/#imperative), starting with a capital. The subject may also contain references to issues or other resources.

The component or topic is typically a few characters long and should always be present. Component names are:

* `doc`: documentation
* `sch`: changes to schematic (PIN mapping, HW connection changes,...)
* `pcb`: changes to device PCB and routing
* `hw` : changes to the design (adding new sensor, IC, etc.)
* `dev`: other non-functional development changes, e.g. Makefile, .gitignore, editor config
* `com `: changes to communication ports (UART, I2C, SPI,...)
* `tmr `: changes to timers, counters components
* `compiler`: compiler related changes (attributes, commands,..)
* `lib`:  libraries (additions, changes, improvements, fixes)
* `app`: application level changes (additions, improvements, fixes)
* `all`: changes affecting all code

Changes that affect multiple components can be comma separated.

Good commit messages:

* `doc: Add README with build instructions`
* `dev: Improve CMAKE build with debug and release options`
* `lib,com,app: Fix flash memory read API`

Make sure that commits are scoped to something meaningful and could, potentially, be merged individually.

### Body

The body may contain a more detailed description of the commit, explaining what it changes and why. The "how" is less relevant, as this should be obvious from the diff.

## Legal

Any contributions related to this repository whether it might be source code improvements or design changes require signing the [Contributor License Agreement (CLA)](https://cla-assistant.io/TheThingsIndustries/generic-node-se).

Please make sure to carefully read the terms of the [CLA](https://cla-assistant.io/TheThingsIndustries/generic-node-se).

All accepted contributions will be a Copyright © of The Things Industries B.V.

[Contact The Things Industries](https://thethingsindustries.com/contact/) for licensing.
