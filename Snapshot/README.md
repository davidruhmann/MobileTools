## Snapshot

This utility will dump output information for all the running processes.  A snapshot of the running processes.  This information includes Process, Heap, Handles, and Windows.

### Notes

**Error C1859** will occur if both SnapshotCE and Snapshot32 use the same OutDir/Target directory.

**LNK1001: Internal error during IncrBuildImage** will occur due to a VS2008 compiler issue.
Install the fix referenced here
[StackOverflow](http://stackoverflow.com/questions/2001289/how-to-resolve-fatal-error-lnk1000-internal-error-during-incrbuildimage)
or [MSDN](http://support.microsoft.com/kb/948127)
or [KB948127](http://archive.msdn.microsoft.com/KB948127)
