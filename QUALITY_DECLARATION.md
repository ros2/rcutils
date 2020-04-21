
This document is a declaration of software quality for the `rcutils` package, based on the guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/rep-2004/rep-2004.rst).

# `rcutils` Quality Declaration

The package `rcutils` claims to be in the **Quality Level 1** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 1 in REP-2004 ](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#quality-level-1) of the ROS2 developer guide.

## Version Policy

### Version Scheme
(Missing part, bumping to version 1.0.0)

`rcutils` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#versioning), and it will be at a stable version, i.e. `>= 1.0.0`.

### API Stability Within a Released ROS Distribution

`rcutils` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution

`rcutils` contains C code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

### Public API Declaration

All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

Additionally, there are generated header files which are installed and therefore part of the public API.
The source templates for these generated headers are in the `resource` folder.

## Change Control Process

`rcutils` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#change-control-process).

This includes:

- all changes occur through a pull request
- all pull request will be peer-reviewed
- all pull request must pass CI on all [tier 1 platforms](https://github.com/ros-infrastructure/rep/blob/master/rep-2000.rst)
- all pull request must resolve related documentation changes before merging

## Documentation

### Feature Documentation

TODO fix link (Missing part)

`rcutils` has a [feature list](TODO) and each item in the list links to the corresponding feature documentation.
There is documentation for all of the features, and new features require documentation before being added.

### Public API Documentation

TODO fix link (Missing part)

`rcutils` has embedded API documentation and it is generated using doxygen and is [hosted](TODO) along side the feature documentation.
There is documentation for all of the public API, and new additions to the public API require documentation before being added.

### License

The license for `rcutils` is Apache 2.0, and a summary is in each source file, the type is declared in the [`package.xml`](https://github.com/ros2/rcutils/blob/master/package.xml) manifest file, and a full copy of the license is in the [`LICENSE`](https://github.com/ros2/rcutils/blob/master/LICENSE) file.

There is an automated test which runs a linter that ensures each file has a license statement.

### Copyright Statements

The copyright holders each provide a statement of copyright in each source code file in `rcutils`.

There is an automated test which runs a linter that ensures each file has at least one copyright statement.

## Testing

### Feature Testing

Each feature in `rcutils` has corresponding tests which simulate typical usage, and they are located in the [`test`](https://github.com/ros2/rcutils/tree/master/test) directory.
New features are required to have tests before being added.

### Public API Testing

Each part of the public API has tests, and new additions or changes to the public API require tests before being added.
The tests aim to cover both typical usage and corner cases, but are quantified by contributing to code coverage.

### Coverage

`rcutils` follows the recommendations for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#code-coverage), and opts to use line coverage instead of branch coverage.

This includes:

- tracking and reporting line coverage statistics
- achieving and maintaining a reasonable branch line coverage (90-100%)
- no lines are manually skipped in coverage calculations

Changes are required to make a best effort to keep or increase coverage before being accepted, but decreases are allowed if properly justified and accepted by reviewers.

Current coverage statistics can be viewed here:

TODO Add link to latest coverage results

### Performance

TODO fix link (Missing part)

`rcutils` follows the recommendations for performance testing of C code in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#performance), and opts to do performance analysis on each release rather than each change.

TODO how to run perf tests, where do they live, etc.

TODO exclusions of parts of the code from perf testing listed here? or linked to?

### Linters and Static Analysis

`rcutils` uses and passes all the ROS2 standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#linters-and-static-analysis). Passing implies there are no linter/static errors when testing against CI of supported platforms.

## Dependencies

`rcutils` has no run-time or build-time dependencies that need to be considered for this declaration.

It has several "buildtool" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.
It also has several test dependencies, which do not affect the resulting quality of the package, because they are only used to build and run the test code.

## Platform Support

`rcutils` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

# Current status Summary

The chart below compares the requirements in the REP-2004 with the current state of the rcutils package.
|Number|  Requirement| Current state |
|--|--|--|
|1| **Version policy** |---|
|1.i|Version Policy available | ✓ |
|1.ii|Stable version |☓|
|1.iii|Declared public API|✓|
|1.iv|API stability policy|✓|
|1.v|ABI stability policy|✓|
|1.vi_|API/ABI stable within ros distribution|✓|
|2| **Change control proces**s |---|
|2.i| All changes occur on change request | ✓|
|2.ii| Contributor origin (DCO, CLA, etc) | ✓|
|2.iii| Peer review policy | ✓ |
|2.iv| CI policy for change requests | ✓ |
|2.v| Documentation policy for change requests | ☓ |
|3| **Documentation** | --- |
|3.i| Per feature documentation | ☓ |
|3.ii| Per public API item documentation | ☓ |
|3.iii| Declared License(s) | ✓ |
|3.iv| Copyright in source files| ✓ |
|3.v.a| Quality declaration linked to README | ✓ |
|3.v.b| Centralized declaration available for peer review |✓|
|4| Testing | --- |
|4.i| Feature items tests | ☓ |
|4.ii| Public API tests | ☓ |
|4.iii.a| Using coverage |✓ |
|4.iii.a| Coverage policy | ✓ |
|4.iv.a| Performance tests (if applicable) | ? |
|4.iv.b| Performance tests policy| ✓ |
|4.v.a| Code style enforcement (linters)| ✓ |
|4.v.b| Use of static analysis tools | ✓ |
|5| Dependencies | --- |
|5.i| Must not have ROS lower level dependencies | ✓ |
|5.ii| Optional ROS lower level dependencies| ✓ |
|5.iii| Justifies quality use of non-ROS dependencies |✓|
|6| Platform support | --- |
|6.i| Support targets Tier1 ROS platforms| ✓ |
|7| Security | --- |
|7.i| Vulnerability Disclosure Policy | ? |

Comparing this table with the [Quality Level Comparison Chart of REP2004](https://github.com/ros-infrastructure/rep/blob/d1074e43f25f957d75f50dbfda94ab10d86bcbfd/rep-2004.rst#quality-level-comparison-chart) lead us to decide that this package qualifies to Quality Level 4. Missing to quality level 3:

1.ii, Stable version
7.i, Vulnerability Disclosure Policy (To be defined in Developer Guide for ROS2 Core packages)
