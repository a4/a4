from glob import glob

def a4_app(year = 2011):
    app = Athena()
    app.option_file = "./pydumpa4.py"
    app.atlas_dbrelease = "LATEST"
    app.options = "-c 'options={\"year\":%i}'" % year
    app.athena_compile = False
    app.prepare()
    return app

def a4_tasks(todo, year = 2011, files_per_job=1, app=None):
    if not app:
        app = a4_app(year)
    tsks = []
    for dsets, name in todo:
        t = AnaTask()
        t.float = 10
        t.name = name
        t.analysis.application = app
        t.analysis.backend = Panda()
        t.analysis.files_per_job = files_per_job
        t.initializeFromDatasets(dsets)
        for tf in t.transforms:
            tf.backend = Panda()
            tf.backend.extOutFile = ["events.a4"]
            tf.backend.nobuild = True
            tf.outputdata.location = "LRZ-LMU_LOCALGROUPDISK"
        tsks.append(t)

    return tsks

def a4_process(datasets_files, year=2011, files_per_job=2, dry_run=False, app=None):
    todo = []
    for datasets_file in datasets_files:
        dss = []
        for ds in open(datasets_file).readlines():
            ds = ds.strip()
            if ds.startswith("#") or not ds:
                continue
            dss.append(ds)
        name = datasets_file.split("/")[-1].replace(".datasets","").replace(".","_")
        todo.append((dss, "a4_%s" % name))
    for dss, name in todo:
        print name
        for ds in dss:
            print " * ", ds
    if dry_run:
        return
    return a4_tasks(todo, year, files_per_job, app)

