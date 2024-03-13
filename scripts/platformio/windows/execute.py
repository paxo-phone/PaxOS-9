Import("env")

env.AddCustomTarget(
    name = "execute",
    title = "Execute Program",
    actions = [
        "\"${PROGPATH}\""
    ],
    dependencies = ["${PROGPATH}"]
)
