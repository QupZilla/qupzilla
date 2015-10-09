import QtQuick 2.2
import QtWebEngine 1.0

WebEngineView {
    width: 1280
    height: 720

    onLoadingChanged: {
        if (loadRequest.status == WebEngineView.LoadStartedStatus)
            return;

        var ok = loadRequest.status == WebEngineView.LoadSucceededStatus;
        thumbnailer.createThumbnail(ok);
    }
}
