function doGet(e) {
    return handleResponse(e);
}

//Recieve parameter and pass it to function to handle

function doPost(e) {
    return handleResponse(e);
}

function handleResponse(request) {
    var output = ContentService.createTextOutput();

    var createAt = new Date();
    var pm25 = request.parameter.pm25;
    var co2 = request.parameter.co2;
    var id = request.parameter.id;

    //open your Spread sheet by passing id
    var ss = SpreadsheetApp.openById(id);
    var sheet = ss.getSheetByName("IAQ");

    if (sheet.getCharts().length === 0) {
        // 清除試算表內容
        sheet.clear();
        // 設定標題列
        sheet.appendRow(["index", "time", "pm2.5", "co2"])
        // 新建圖表
        var embededChartBuilder = sheet.newChart().asLineChart();
        var chart = embededChartBuilder
            // 使用第一列作為標題
            .setOption("useFirstColumnAsDomain", true)
            /* 設定X軸資料來源 */
            .addRange(sheet.getRange("B:B"))
            /* 設定Y軸資料來源 */
            .addRange(sheet.getRange("C:C"))
            .addRange(sheet.getRange("D:D"))
            /* 設定圖表長寬 */
            .setOption('width', 605)
            .setOption('height', 420)
            .setNumHeaders(1)
            /* 設定圖表繪製位置 */
            .setPosition(1, 5, 0, 0)
            // 設定標題
            .setOption("title", "IAQ")
            // 設定x軸標題
            .setOption("hAxis", { title: "時間" })
            .setOption("legend", { position: "top" })
            /* 執行上面所有設定 */
            .build();
        sheet.insertChart(chart);
    }
    sheet.appendRow([sheet.getLastRow(), createAt, pm25, co2])
    var cur_pm25_cell = sheet.getRange("C" + sheet.getLastRow());
    var cur_co2_cell = sheet.getRange("D" + sheet.getLastRow());

    if (cur_pm25_cell.getValue() > 53) {
        cur_pm25_cell.setBackground("#e06666");
    } else if (cur_pm25_cell.getValue() > 35) {
        cur_pm25_cell.setBackground("#f6b26b");
    } else {
        cur_pm25_cell.setBackground("#6aa84f");
    }

    if (cur_co2_cell.getValue() > 1000) {
        cur_co2_cell.setBackground("#e06666");
    } else if (cur_co2_cell.getValue() > 700) {
        cur_co2_cell.setBackground("#f6b26b");
    } else {
        cur_co2_cell.setBackground("#6aa84f");
    }

    var callback = request.parameters.callback;
    if (callback === undefined) {
        output.setContent(JSON.stringify("Success"));
    } else {
        output.setContent(callback + "(" + JSON.stringify("Success") + ")");
    }

    output.setMimeType(ContentService.MimeType.JSON);

    return output;
}