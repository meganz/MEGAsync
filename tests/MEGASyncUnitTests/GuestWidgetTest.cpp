#include <catch.hpp>
#include <trompeloeil.hpp>
#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include <QMessageBox>
#include <thread>
#include "QTMegaRequestListener.h"

using namespace std::literals::string_literals;

class MegaApiMock : public mega::MegaApi
{
public:
    MegaApiMock():mega::MegaApi("appKey"){};
    MAKE_MOCK1(addRequestListener, void(mega::MegaRequestListener* listener), override);
    MAKE_MOCK3(login, void(const char* email, const char* password, mega::MegaRequestListener *listener), override);
};

QMessageBox* waitUntilVisibleMessageBox()
{
    QMessageBox* messageBox{nullptr};
    while(!messageBox)
    {
        const auto topWidgets{QApplication::topLevelWidgets()};
        for (const auto widget : topWidgets)
        {
            messageBox = qobject_cast<QMessageBox*>(widget);
            if(messageBox)
            {
                break;
            }
        }
        if(!messageBox)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
    while(!messageBox->isVisible())
    {
       std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return messageBox;
}

SCENARIO("User login")
{
    GIVEN("A Guest Widget form")
    {
        const auto megaApiMock{std::make_unique<MegaApiMock>()};
        REQUIRE_CALL(*megaApiMock, addRequestListener(trompeloeil::_));
        GuestWidget guestWidget(megaApiMock.get());

        THEN("Current widget is the login page")
        {
            const auto stackedPages{guestWidget.findChild<QStackedWidget*>(QStringLiteral("sPages"))};
            const auto pageLogin{guestWidget.findChild<QWidget*>(QStringLiteral("pLogin"))};
            REQUIRE(stackedPages->currentWidget() == pageLogin);

            const auto buttonLogin{guestWidget.findChild<QPushButton*>(QStringLiteral("bLogin"))};
            const auto stackedLoginTitles{guestWidget.findChild<QStackedWidget*>(QStringLiteral("sLoginTitle"))};
            const auto pageLoginErrors{guestWidget.findChild<QWidget*>(QStringLiteral("pLoginErrors"))};
            const auto labelLoginErrors{guestWidget.findChild<QLabel*>(QStringLiteral("lLoginErrors"))};
            const auto lineEditEmail{guestWidget.findChild<QLineEdit*>(QStringLiteral("lEmail"))};

            AND_THEN("An error is shown if login button is clicked without filling user credentials")
            {
                buttonLogin->click();
                REQUIRE(stackedLoginTitles->currentWidget() == pageLoginErrors);
                REQUIRE(labelLoginErrors->text() == QStringLiteral("Please, enter your e-mail address"));
            }

            AND_WHEN("An error is shown if login button is clicked with filling a wrong email address")
            {
                lineEditEmail->setText(QStringLiteral("wrongEmailAddress"));
                buttonLogin->click();
                REQUIRE(stackedLoginTitles->currentWidget() == pageLoginErrors);
                REQUIRE(labelLoginErrors->text() == QStringLiteral("Please, enter a valid e-mail address"));

                lineEditEmail->setText(QStringLiteral("wrongEmailAddress@"));
                buttonLogin->click();
                REQUIRE(stackedLoginTitles->currentWidget() == pageLoginErrors);
                REQUIRE(labelLoginErrors->text() == QStringLiteral("Please, enter a valid e-mail address"));
            }

            AND_WHEN("A correct email is entered")
            {
                const auto email{"email@email.com"s};
                lineEditEmail->setText(QString::fromStdString(email));

                THEN("An error dialog is thrown if password is not filled")
                {
                    buttonLogin->click();
                    REQUIRE(stackedLoginTitles->currentWidget() == pageLoginErrors);
                    REQUIRE(labelLoginErrors->text() == QStringLiteral("Please, enter your password"));
                }

                AND_WHEN("A password is entered")
                {
                    const auto password{"myPassword"s};
                    const auto lineEditPassword{guestWidget.findChild<QLineEdit*>(QStringLiteral("lPassword"))};
                    lineEditPassword->setText(QString::fromStdString(password));

                    THEN("The loggin process is launched when login button is clicked")
                    {
                        REQUIRE_CALL(*megaApiMock, login(trompeloeil::eq<const char*>(email),
                                           trompeloeil::eq<const char*>(password), nullptr));
                        buttonLogin->click();
                    }
                }
            }
        }
    }
}
