from glob import glob

def gangatasks(todo, year = 2011):
    app = Athena()
    app.option_file = "./run.py"
    app.atlas_dbrelease = "LATEST"
    app.options = "-c 'options={\"year\":%i}'" % year
    app.athena_compile = False
    app.prepare()
    tsks = []
    for dsets, name in todo:
        t = AnaTask()
        t.float = 10
        t.name = name
        t.analysis.application = app
        t.analysis.backend = Panda()
        t.analysis.files_per_job = 1
        t.initializeFromDatasets(dsets)
        for tf in t.transforms:
            tf.backend = Panda()
            tf.backend.extOutFile = ["events.a4"]
            tf.backend.nobuild = True
        tsks.append(t)

    return tsks
