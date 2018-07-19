# go-lazycache-appengine

This is the package for deploying the
[go-lazycache](https://github.com/amarburg/go-lazycache) application to [Google
App Engine.](https://cloud.google.com/appengine/)   We use the custom runtime
functionality where the deployment package is basically a Dockerfile plus the
vendor-specific `app.yaml`  configuration file.
