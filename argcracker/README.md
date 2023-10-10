# argcracker

This can crack any SqexArg formatted string you throw at it. However there are two
caveats:

1. You must know the current TickCount() when you got the string, the easiest way is to run your victim program and then run argcracker right after.
2. You must know at least one known argument (such as `UserPath`) or the decrypted result will be hard to find.

## Usage

```bash
$ novus-argcracker [sqexarg string] [tick range] [known arg]`
```

Example:

```bash
`$ novus-argcracker //**sqex0003p8LrsXt9_m9RJAsGzXd66zb3SxeTqZdhV**// 1000 UserPath`
```